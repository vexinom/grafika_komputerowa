#version 330 core

out vec4 FragColor;

in float Height;
in vec3 Position;
in vec3 WorldNormal;
in vec2 TexCoord;
in vec3 WorldPos; 

uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform vec3 cameraPos;

uniform vec3 sunDirection;

void main()
{
    vec4 sandColor = texture(sandTexture, TexCoord);
    vec4 grassColor = texture(grassTexture, TexCoord); 

    float blendFactor = smoothstep(100.0, 110.0, Height);
    vec3 mixedTerrainColor = mix(sandColor.rgb, grassColor.rgb, blendFactor);

    vec3 norm = normalize(WorldNormal);
    vec3 sunDir = normalize(sunDirection);

    norm = normalize(mix(norm, vec3(0.0, 1.0, 0.0), 0.5));

    float dotNL = max(dot(norm, sunDir), 0.0);

    float sunIntensity = smoothstep(-0.1, 0.1, sunDir.y);
    float diffuseIntensity = dotNL * sunIntensity;

    
    vec3 ambientDay = vec3(0.3, 0.3, 0.3);
    vec3 ambientNight = vec3(0.05, 0.1, 0.25);

    vec3 ambientColor = mix(ambientNight, ambientDay, sunIntensity);

    vec3 finalLight = mixedTerrainColor * (vec3(1.0) * diffuseIntensity + ambientColor);
    
    FragColor = vec4(finalLight, 1.0);
}