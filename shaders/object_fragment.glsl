#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform vec3 sunDirection;
uniform vec3 cameraPos;
uniform sampler2D shadowMap;

float ShadowFactor(float NdotL)
{
    vec3 proj = FragPosLightSpace.xyz / FragPosLightSpace.w * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0)
        return 0.0;

    float bias = max(0.012 * (1.0 - NdotL), 0.0012);
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

    return shadow / 9.0;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(sunDirection);
    vec3 V = normalize(cameraPos - FragPos);
    vec3 H = normalize(L + V);

    float sunIntensity = smoothstep(-0.1, 0.1, L.y);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = ShadowFactor(NdotL);

    float diffuse = NdotL * sunIntensity * (1.0 - shadow);
    float specular = pow(max(dot(N, H), 0.0), 32.0) * sunIntensity * (1.0 - 0.55 * shadow);

    vec3 baseColor = vec3(0.55, 0.55, 0.58);
    vec3 color = baseColor * (0.18 + diffuse) + vec3(1.0) * specular * 0.25;

    FragColor = vec4(color, 1.0);
}
