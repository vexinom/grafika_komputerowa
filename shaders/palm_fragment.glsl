#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform sampler2D albedo;
uniform vec3 sunDirection;
uniform vec3 cameraPos;

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
    float spec = pow(max(dot(N, H), 0.0), 24.0) * 0.05;

    vec3 ambient = tex.rgb * mix(vec3(0.18, 0.22, 0.20), vec3(0.34, 0.38, 0.32), sunIntensity);
    vec3 diffuse = tex.rgb * vec3(1.0, 0.96, 0.86) * NdotL * sunIntensity;

    vec3 color = ambient + diffuse + vec3(spec);

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, tex.a);
}
