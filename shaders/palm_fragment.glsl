#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1; 
uniform sampler2D shadowMap;

uniform vec3 sunDirection;
uniform vec3 cameraPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0)
        return 0.0;
        
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
   
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    
    return shadow;
}

void main()
{    
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    if(texColor.a < 0.1)
        discard;

    texColor.rgb = mix(texColor.rgb, vec3(0.192, 0.78, 0.278), 0.25);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sunDirection);

   
    float sunHeight = lightDir.y; 
    float dayFactor = clamp(sunHeight * 2.0, 0.0, 1.0); 
  
    vec3 ambient = (vec3(0.25) + vec3(0.2) * dayFactor) * texColor.rgb;

    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (vec3(0.8) * diff * dayFactor) * texColor.rgb; 

    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);                      
    
    vec3 finalColor = ambient + (1.0 - shadow) * diffuse;
    
    FragColor = vec4(finalColor, texColor.a);
}