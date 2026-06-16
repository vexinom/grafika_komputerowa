#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 sunDirection;
uniform vec3 cameraPos;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(sunDirection);
    vec3 V = normalize(cameraPos - FragPos);
    vec3 H = normalize(L + V);

    float sunIntensity = smoothstep(-0.1, 0.1, L.y);
    float diffuse = max(dot(N, L), 0.0) * sunIntensity;
    float specular = pow(max(dot(N, H), 0.0), 48.0) * sunIntensity;

    vec3 baseColor = vec3(0.90, 0.45, 0.12);
    vec3 color = baseColor * (0.25 + diffuse) + vec3(1.0) * specular * 0.5;

    FragColor = vec4(color, 1.0);
}
