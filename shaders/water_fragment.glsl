#version 330 core

in vec4 v_color;
in vec3 FragPos; 
in vec3 Normal;  

out vec4 FragColor;


uniform vec3 viewPos;  
uniform vec3 sunDirection; 

void main()
{

    vec4 waterBaseColor = vec4(0.1, 0.4, 0.8, 0.6); 
    vec3 albedo = waterBaseColor.rgb * v_color.rgb * 2.5; 

    vec3 sunDir = normalize(sunDirection);
    float sunIntensity = smoothstep(-0.1, 0.1, sunDir.y);

    vec3 ambientDay = vec3(0.3, 0.3, 0.3);
    vec3 ambientNight = vec3(0.11, 0.114, 0.122);
    vec3 ambientColor = mix(ambientNight, ambientDay, sunIntensity);
    vec3 ambient = ambientColor * albedo;

    vec3 norm = normalize(Normal);
    
    float diff = max(dot(norm, sunDir), 0.0);
    vec3 diffuse = diff * albedo * 0.3 * sunIntensity; 

    float specularStrength = 1.5; 
    vec3 viewDirection = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(sunDir + viewDirection);  
    
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);

    vec3 specular = specularStrength * spec * vec3(1.0, 0.9, 0.7) * sunIntensity; 

    vec3 finalColor = ambient + diffuse + specular;
    
    float distance = length(viewPos - FragPos);


    float fogMin = 800.0;  
    float fogMax = 1600.0; 
    
    float fogFactor = smoothstep(fogMin, fogMax, distance);

    vec3 horizonDay = vec3(0.278, 0.757, 0.922);
    vec3 horizonSunset = vec3(0.9, 0.4, 0.1);
    vec3 horizonNight = vec3(0.086, 0.086, 0.09);

    vec3 currentHorizon = mix(horizonSunset, horizonDay, clamp(sunDir.y + 0.2, 0.0, 1.0));
    if(sunDir.y < 0.0) 
    {
        currentHorizon = mix(horizonNight, horizonSunset, clamp(sunDir.y + 1.0, 0.0, 1.0));
    }

    finalColor = mix(finalColor, currentHorizon, fogFactor);

    FragColor = vec4(finalColor, waterBaseColor.a);
}