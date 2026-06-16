#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
in vec4 FragPosLightSpace;

uniform sampler2D baseColor;
uniform sampler2D opacity;
uniform sampler2D shadowMap;

uniform vec3 sunDirection;
uniform vec3 cameraPos;
uniform vec3 headlightPos;
uniform vec3 headlightColor;

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
    if (texture(opacity, UV).r < 0.5) discard;

    vec3 albedo = texture(baseColor, UV).rgb;
    vec3 N = normalize(Normal);
    vec3 L = normalize(sunDirection);
    vec3 V = normalize(cameraPos - FragPos);
    vec3 H = normalize(L + V);

    float sunIntensity = smoothstep(-0.1, 0.1, L.y);
    float NdotL = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 24.0) * sunIntensity;
    float shadow = ShadowFactor(NdotL);

    vec3 ambient = albedo * mix(vec3(0.10), vec3(0.30), sunIntensity);
    vec3 color = ambient + (albedo * NdotL + vec3(0.25) * spec) * sunIntensity * (1.0 - shadow);

    vec3 toLight = headlightPos - FragPos;
    float d = length(toLight);
    float att = 1.0 / (1.0 + 0.0006 * d * d);
    color += albedo * headlightColor * max(dot(N, toLight / d), 0.0) * att;

    FragColor = vec4(color, 1.0);
}
