#version 330 core

out vec4 FragColor;

in float Height;
in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in vec3 WorldPos; 

uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform vec3 cameraPos;

void main()
{
    vec4 sandColor = texture(sandTexture, TexCoord);
    vec4 grassColor = texture(grassTexture, TexCoord); 

    float blendFactor = smoothstep(100.0, 110.0, Height);
    vec3 mixedTerrainColor = mix(sandColor.rgb, grassColor.rgb, blendFactor);

    vec3 lightDirection = normalize(vec3(0.5, 1.0, 0.3));
    vec3 norm = normalize(Normal);
    norm = normalize(mix(norm, vec3(0.0, 1.0, 0.0), 0.5));

    float dotNL = dot(norm, lightDirection);
    float diffuseIntensity = pow(dotNL * 0.5 + 0.5, 2.0);
    float ambientIntensity = 0.1;

    vec3 finalLight = mixedTerrainColor * (diffuseIntensity + ambientIntensity);

    vec4 finalColor = vec4(finalLight, 1.0);

    float waterHeight = 90.0;

    

    FragColor = finalColor;
}