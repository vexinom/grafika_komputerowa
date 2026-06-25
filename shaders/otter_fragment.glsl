#version 330 core
in vec3 vNormal; in vec2 vUV;
out vec4 FragColor;
uniform sampler2D albedo;
uniform vec3 sunDirection;
uniform vec3 tint;
void main() {
    vec3 N = normalize(vNormal);
    if (!gl_FrontFacing) N = -N;          // double-sided fur: light both faces correctly
    vec3 L = normalize(sunDirection);

    float sunIntensity = smoothstep(-0.1, 0.1, L.y);
    float NdotL = max(dot(N, L), 0.0);

    vec3 alb = texture(albedo, vUV).rgb * tint;

    vec3 color = alb * vec3(1.0, 0.97, 0.9) * sunIntensity * NdotL;   // key light
    color += alb * mix(vec3(0.14), vec3(0.34), sunIntensity);         // ambient

    color = color / (color + vec3(1.0));     // tonemap (same as axolotl/terrain)
    color = pow(color, vec3(1.0 / 2.2));     // gamma to sRGB
    FragColor = vec4(color, 1.0);
}
