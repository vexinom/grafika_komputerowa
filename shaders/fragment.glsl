#version 330 core

out vec4 FragColor;

in float Height;
in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D surfaceTexture;

void main()
{
    vec4 texColor = texture(surfaceTexture, TexCoord);

    vec3 lightDirection = normalize(vec3(0.5, 1.0, 0.3));
    vec3 norm = normalize(Normal);

    norm = normalize(mix(norm, vec3(0.0, 1.0, 0.0), 0.5));

    float dotNL = dot(norm, lightDirection);
    float diffuseIntensity = pow(dotNL * 0.5 + 0.5, 2.0);

    float ambientIntensity = 0.1;

    vec3 finalLight = texColor.rgb * (diffuseIntensity + ambientIntensity);

    FragColor = vec4(finalLight, 1.0);
}