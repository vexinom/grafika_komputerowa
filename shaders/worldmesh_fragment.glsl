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

// procedural detail to break up the low-res tiled ground texture
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

float ShadowFactor(float NdotL, vec3 fragPos, vec3 viewPos)
{
    // 1. Sprawdzenie odległości od kamery (1500.0 jednostek)
    float distance = length(viewPos - fragPos);
    if (distance > 1500.0) return 1.0; // Poza zasięgiem - brak cienia

    // 2. Konwersja do przestrzeni tekstury
    vec3 proj = FragPosLightSpace.xyz / FragPosLightSpace.w * 0.5 + 0.5;
    
    // Jeśli poza zasięgiem Z, nie ma cienia
    if (proj.z < 0.0 || proj.z > 1.0) return 1.0;

    float bias = max(0.0025 * (1.0 - NdotL), 0.0008);
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
    shadow /= 9.0;

    // 3. Płynne wygaszanie poza mapą cieni
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0) return 1.0; 

    return 1.0 - shadow;
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

    // procedural grain so the ground doesn't look like a blurry flat sheet
    

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

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float D = DistributionGGX(N, H, roughness * roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = D * G * F / max(4.0 * max(dot(N, V), 0.0) * NdotL, 0.001);
    vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);

    float shadow = ShadowFactor(NdotL);
    vec3 direct = (kd * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);
    vec3 ambient = albedo * mix(vec3(0.10), vec3(0.32), sunIntensity);

    vec3 color = ambient + direct;

    vec3 toHeadlight = headlightPos - WorldPos;
    float hd = length(toHeadlight);
    float hatt = 1.0 / (1.0 + 0.0006 * hd * hd);
    if (Height < water_level)
    {
        color += albedo * headlightColor * max(dot(N, toHeadlight / hd), 0.0) * hatt;
    }
    
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
