#version 330 core
out vec4 FragColor;

in float Height;
in vec2 TexCoord;
in vec3 WorldPos;
in vec4 FragPosLightSpace;
in mat3 TBN;

uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform sampler2D sandNormal;
uniform sampler2D grassNormal;
uniform sampler2D shadowMap;

uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 cameraPos;
uniform float metallic;
uniform float roughness;
uniform vec3 headlightPos;
uniform vec3 headlightColor;
uniform float water_level;

const float PI = 3.14159265359;

float h2(vec2 p){ return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }
float vn2(vec2 p){
    vec2 i = floor(p), f = fract(p); f = f * f * (3.0 - 2.0 * f);
    return mix(mix(h2(i), h2(i + vec2(1,0)), f.x), mix(h2(i + vec2(0,1)), h2(i + vec2(1,1)), f.x), f.y);
}
float fbm2(vec2 p){ return 0.5 * vn2(p) + 0.25 * vn2(p * 2.07) + 0.125 * vn2(p * 4.13); }

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float r)
{
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    float nv = max(dot(N, V), 0.0);
    float nl = max(dot(N, L), 0.0);
    float gv = nv / (nv * (1.0 - k) + k);
    float gl = nl / (nl * (1.0 - k) + k);
    return gv * gl;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float ShadowFactor(float NdotL)
{
    vec3 proj = FragPosLightSpace.xyz / FragPosLightSpace.w * 0.5 + 0.5;

    if (proj.z > 1.0 || proj.z < 0.0 ||
        proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 0.0;

    // Depth range is now tight (~2500 units) and acne is handled by polygon
    // offset in the depth pass, so the receiver bias can stay tiny. A large bias
    // here is what used to detach (peter-pan) the lighthouse shadow from its base.
    float bias = max(0.0010 * (1.0 - NdotL), 0.0004);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, proj.xy + vec2(x, y) * texelSize).r;
            shadow += (proj.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    return min(shadow / 9.0, 0.75);
}

void main()
{
    vec3 sandCol  = texture(sandTexture, TexCoord).rgb;
    vec3 grassCol = texture(grassTexture, TexCoord).rgb;
    vec3 deepCol  = mix(sandCol, vec3(0.10, 0.13, 0.12), 0.7) * 0.65;

    float sandStart = water_level - 5.0; 
    float sandEnd   = water_level + 5.0; 
    
    float grassStart = sandEnd;
    float grassEnd   = sandEnd + 20.0; 

    float toSand  = smoothstep(sandStart, sandEnd, Height);
    float toGrass = smoothstep(grassStart, grassEnd, Height);
    vec3 albedo = mix(deepCol, sandCol, toSand);
    albedo = mix(albedo, grassCol, toGrass);

    float grain = 0.6 * fbm2(WorldPos.xz * 0.5) + 0.4 * fbm2(WorldPos.xz * 4.0);
    albedo *= (0.82 + 0.36 * grain);

    vec3 tangentNormal = mix(texture(sandNormal, TexCoord).xyz, texture(grassNormal, TexCoord).xyz, toGrass) * 2.0 - 1.0;
    vec3 N = normalize(TBN * tangentNormal);
    vec3 V = normalize(cameraPos - WorldPos);
    vec3 L = normalize(sunDirection);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float sunIntensity = smoothstep(-0.1, 0.1, L.y);
    vec3 radiance = sunColor * sunIntensity;

    // Slight local roughness variation from terrain grain gives visible glints/spots.
    float roughnessLocal = clamp(roughness * mix(0.60, 1.25, grain), 0.04, 1.0);
    float glintMask = smoothstep(0.74, 0.98, grain) * (1.0 - roughnessLocal);

    // Terrain albedo is not metallic by nature, so avoid tinting F0 too strongly by albedo.
    vec3 dielectricF0 = vec3(0.04);
    vec3 metalTint = mix(vec3(0.62), albedo, 0.35);
    vec3 F0 = mix(dielectricF0, metalTint, metallic);

    float D = DistributionGGX(N, H, roughnessLocal * roughnessLocal);
    float G = GeometrySmith(N, V, L, roughnessLocal);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (D * G * F / max(4.0 * max(dot(N, V), 0.0) * NdotL, 0.001)) * 1.45;
    vec3 kd = (vec3(1.0) - F) * (1.0 - 0.75 * metallic);

    float shadow = ShadowFactor(NdotL);
    float diffuseVisibility = 1.0 - shadow;
    float specVisibility = 1.0 - 0.45 * shadow;
    vec3 directDiffuse = kd * albedo / PI * radiance * NdotL * diffuseVisibility;
    vec3 directSpecular = specular * radiance * NdotL * specVisibility;
    vec3 direct = directDiffuse + directSpecular;

    // Extra high-frequency sun glints so highlights are clearly visible on terrain.
    float sunMirror = pow(max(dot(reflect(-L, N), V), 0.0), mix(18.0, 240.0, 1.0 - roughnessLocal));
    vec3 sunGlints = sunColor * sunIntensity * (0.25 + 1.9 * metallic) * sunMirror * (0.25 + 2.2 * glintMask) * specVisibility;

    vec3 ambientDay = vec3(0.3, 0.3, 0.3);
    vec3 ambientNightUnderwater = vec3(0.15, 0.15, 0.25); 
    vec3 ambientNightAboveWater = vec3(0.02, 0.02, 0.04);

    float aboveWaterMask = smoothstep(water_level - 1.0, water_level + 1.0, Height);
    vec3 currentAmbientNight = mix(ambientNightUnderwater, ambientNightAboveWater, aboveWaterMask);

    vec3 ambientColor = mix(currentAmbientNight, ambientDay, sunIntensity);

    // Ambient PBR approximation so metallic/roughness still influence the terrain
    // when direct sunlight is weak or shadowed.
    float NdotV = max(dot(N, V), 0.0);
    vec3 F_ambient = FresnelSchlick(NdotV, F0);
    vec3 kdAmbient = (vec3(1.0) - F_ambient) * (1.0 - 0.70 * metallic);
    vec3 ambientDiffuse = kdAmbient * albedo * ambientColor;
    vec3 ambientSpecular = F_ambient * ambientColor * mix(0.36, 0.06, roughnessLocal);
    vec3 ambient = ambientDiffuse + ambientSpecular;

    vec3 color = ambient + direct + sunGlints;

    vec3 toHeadlight = headlightPos - WorldPos;
    float hd = length(toHeadlight);
    float hatt = 1.0 / (1.0 + 0.0006 * hd * hd);
    if (Height < water_level && hd > 0.001)
    {
        vec3 Lh = toHeadlight / hd;
        vec3 Hh = normalize(V + Lh);
        float NdotLh = max(dot(N, Lh), 0.0);
        vec3 Fh = FresnelSchlick(max(dot(Hh, V), 0.0), F0);
        float Dh = DistributionGGX(N, Hh, roughnessLocal * roughnessLocal);
        float Gh = GeometrySmith(N, V, Lh, roughnessLocal);
        vec3 specHeadlight = (Dh * Gh * Fh / max(4.0 * max(dot(N, V), 0.0) * NdotLh, 0.001)) * 1.55;
        vec3 kdHeadlight = (vec3(1.0) - Fh) * (1.0 - 0.75 * metallic);

        float headMirror = pow(max(dot(reflect(-Lh, N), V), 0.0), mix(14.0, 180.0, 1.0 - roughnessLocal));
        vec3 headGlints = headlightColor * (0.35 + 2.4 * metallic) * headMirror * (0.25 + 2.0 * glintMask) * NdotLh * hatt;

        color += (kdHeadlight * albedo / PI + specHeadlight) * headlightColor * NdotLh * hatt;
        color += headGlints;
    }
    
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}