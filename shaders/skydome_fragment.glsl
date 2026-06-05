#version 330 core

out vec4 FragColor;

in vec3 LocalPos;
uniform vec3 sunDirection;

void main()
{
    
    vec3 zenithColor = vec3(0.05, 0.15, 0.45); 
    vec3 horizonColor = vec3(0.278, 0.757, 0.922); 


    float gradientFactor = clamp(LocalPos.y, 0.0, 1.0);
    vec3 finalColor = mix(horizonColor, zenithColor, gradientFactor);

    vec3 viewDir = normalize(LocalPos);
    vec3 sunDir = normalize(sunDirection);

    float sunAngle = max(dot(viewDir, sunDir), 0.0);

    float sunDisc = smoothstep(0.995, 0.998, sunAngle);

    float sunGlow = pow(sunAngle, 64.0) * 0.4;
    vec3 sunColor = vec3(1.0, 0.95, 0.8);

    finalColor += sunColor * sunDisc;
    finalColor += sunColor * sunGlow;

    FragColor = vec4(finalColor, 1.0);
}