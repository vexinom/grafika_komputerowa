#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
in vec4 FragPosLightSpace;

uniform vec3 sunDirection;
uniform vec3 cameraPos;
uniform sampler2D albedoMap;
uniform sampler2D shadowMap;

float ShadowFactor(float NdotL)
{
    vec3 proj = FragPosLightSpace.xyz / FragPosLightSpace.w * 0.5 + 0.5;

    if (proj.z > 1.0 || proj.z < 0.0 ||
        proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 0.0;

    float bias = max(0.0015 * (1.0 - NdotL), 0.0005);
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
    vec3 albedo = texture(albedoMap, UV).rgb;

    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - FragPos);
    // Thin, double-sided wings: face the normal toward the viewer.
    if (dot(N, V) < 0.0) N = -N;

    vec3 L = normalize(sunDirection);
    vec3 H = normalize(L + V);

    float sun = smoothstep(-0.1, 0.1, L.y);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = ShadowFactor(NdotL);

    float diffuse  = NdotL * sun * (1.0 - shadow);
    float specular = pow(max(dot(N, H), 0.0), 24.0) * sun * (1.0 - shadow) * 0.12;
    float rim      = pow(1.0 - max(dot(N, V), 0.0), 3.0) * 0.25 * sun;

    // Sky-tinted ambient by day, dim blue at night.
    vec3 ambient = mix(vec3(0.10, 0.12, 0.16), vec3(0.42, 0.46, 0.52), sun);

    vec3 color = albedo * (ambient + diffuse)
               + vec3(1.0) * specular
               + vec3(0.80, 0.85, 1.0) * rim;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
