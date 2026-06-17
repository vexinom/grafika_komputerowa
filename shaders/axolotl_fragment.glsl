#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
in vec4 FragPosLightSpace;

uniform sampler2D baseColor;
uniform sampler2D opacity;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D shadowMap;

uniform vec3 sunDirection;
uniform vec3 cameraPos;
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

vec3 PBR(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, float metallic, float roughness)
{
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float D = DistributionGGX(N, H, roughness * roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = D * G * F / max(4.0 * max(dot(N, V), 0.0) * NdotL, 0.001);
    vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);
    return (kd * albedo / PI + specular) * radiance * NdotL;
}

void main()
{
    if (texture(opacity, UV).r < 0.5) discard;

    vec3 albedo    = texture(baseColor, UV).rgb;
    float metallic = texture(metallicMap, UV).r;
    float roughness = clamp(texture(roughnessMap, UV).r, 0.08, 1.0);

    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - FragPos);
    vec3 L = normalize(sunDirection);

    float sunIntensity = smoothstep(-0.1, 0.1, L.y);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = ShadowFactor(NdotL);

    // sun (key light)
    vec3 radiance = vec3(1.0, 0.97, 0.9) * sunIntensity;
    vec3 color = PBR(N, V, L, radiance, albedo, metallic, roughness) * (1.0 - shadow);

    // headlight (point light following the lead creature)
    vec3 toLight = headlightPos - FragPos;
    float d = length(toLight);
    vec3 Lh = toLight / max(d, 0.001);
    float att = 1.0 / (1.0 + 0.0006 * d * d);
    color += PBR(N, V, Lh, headlightColor * att, albedo, metallic, roughness);

    // ambient
    color += albedo * mix(vec3(0.10), vec3(0.30), sunIntensity);

    // tonemap + gamma to sRGB (consistent with the terrain shader)
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
