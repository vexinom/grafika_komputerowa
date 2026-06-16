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

const float PI = 3.14159265359;

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
    if (proj.z > 1.0) return 0.0;

    float bias = max(0.0025 * (1.0 - NdotL), 0.0008);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
            shadow += proj.z - bias > texture(shadowMap, proj.xy + vec2(x, y) * texelSize).r ? 1.0 : 0.0;

    return shadow / 9.0;
}

void main()
{
    float blendFactor = smoothstep(100.0, 110.0, Height);
    vec3 albedo = mix(texture(sandTexture, TexCoord).rgb, texture(grassTexture, TexCoord).rgb, blendFactor);

    vec3 tangentNormal = mix(texture(sandNormal, TexCoord).xyz, texture(grassNormal, TexCoord).xyz, blendFactor) * 2.0 - 1.0;
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
    vec3 ambient = albedo * mix(vec3(0.05), vec3(0.25), sunIntensity);

    vec3 color = ambient + direct;

    vec3 toHeadlight = headlightPos - WorldPos;
    float hd = length(toHeadlight);
    float hatt = 1.0 / (1.0 + 0.0006 * hd * hd);
    color += albedo * headlightColor * max(dot(N, toHeadlight / hd), 0.0) * hatt;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
