#version 330 core

out vec4 FragColor;

in float Height;
in vec3 Position;
in vec3 Normal;

void main()
{
    vec3 sandColor = vec3(0.76, 0.70, 0.50); 
    vec3 islandColor = vec3(0.122, 0.439, 0.149);  

    float sandLevel = 45.0;   
    float islandLevel = 65.0; 

  
    float blendFactor = smoothstep(sandLevel, islandLevel, Height);

    vec3 terrainColor = mix(sandColor, islandColor, blendFactor);

    vec3 lightDirection = normalize(vec3(0.5, 1.0, 0.3));
    vec3 norm = normalize(Normal);
    norm = normalize(mix(norm, vec3(0.0, 1.0, 0.0), 0.2));

    float diffuseIntensity = max(dot(norm, lightDirection), 0.0);
    float ambientIntensity = 0.2;

    vec3 finalLight = terrainColor * (diffuseIntensity + ambientIntensity);

    FragColor = vec4(finalLight, 1.0);
}