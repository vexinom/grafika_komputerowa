#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;
in vec4 vFragPosLightSpace;

out vec4 FragColor;

uniform sampler2D albedo;
uniform sampler2D shadowMap;
uniform vec3 sunDirection;
uniform vec3 cameraPos;

float ShadowFactor(float NdotL)
{
    vec3 proj = vFragPosLightSpace.xyz / vFragPosLightSpace.w * 0.5 + 0.5;

    if (proj.z > 1.0 || proj.z < 0.0 ||
        proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 0.0;

    // Tight depth range + polygon-offset acne removal -> small bias is enough.
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

    return min(shadow / 9.0, 0.85);
}

void main()
{
    vec4 tex = texture(albedo, vUV);

    if (tex.a < 0.08)
        discard;

    vec3 N = normalize(vNormal);
    if (!gl_FrontFacing)
        N = -N;

    vec3 L = normalize(sunDirection);
    vec3 V = normalize(cameraPos - vFragPos);
    vec3 H = normalize(L + V);

    float sunIntensity = smoothstep(-0.1, 0.15, L.y);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = ShadowFactor(NdotL);
    float spec = pow(max(dot(N, H), 0.0), 24.0) * 0.05 * (1.0 - 0.6 * shadow);

    vec3 ambient = tex.rgb * mix(vec3(0.18, 0.22, 0.20), vec3(0.34, 0.38, 0.32), sunIntensity);
    vec3 diffuse = tex.rgb * vec3(1.0, 0.96, 0.86) * NdotL * sunIntensity * (1.0 - shadow);

    vec3 color = ambient + diffuse + vec3(spec);

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, tex.a);
}
