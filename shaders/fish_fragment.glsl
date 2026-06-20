#version 330 core
in vec3 vNormal; in vec2 vUV;
out vec4 FragColor;
uniform sampler2D albedo;     
uniform vec3 sunDirection;
uniform vec3 tint;            
void main() {
    vec3 N = normalize(vNormal);
    float diff = max(dot(N, normalize(sunDirection)), 0.0);
    vec3 base = texture(albedo, vUV).rgb * tint;   
    float light = 0.55 + 0.6 * diff;
    FragColor = vec4(clamp(base * light, 0.0, 1.0), 1.0);
}
