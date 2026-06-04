#version 330 core

out vec4 FragColor;

in vec3 LocalPos;

void main()
{
    
    vec3 zenithColor = vec3(0.05, 0.15, 0.45); 
    vec3 horizonColor = vec3(0.278, 0.757, 0.922); 


    float gradientFactor = clamp(LocalPos.y, 0.0, 1.0);
    vec3 finalColor = mix(horizonColor, zenithColor, gradientFactor);

    FragColor = vec4(finalColor, 1.0);
}