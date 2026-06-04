#version 330 core

in vec4 v_color;
in vec3 FragPos; 
in vec3 Normal;  

out vec4 FragColor;


uniform vec3 viewPos;  
uniform vec3 lightDir; 

void main()
{

    vec4 waterBaseColor = vec4(0.1, 0.4, 0.8, 0.6); 
    vec3 albedo = waterBaseColor.rgb * v_color.rgb * 2.5; 

    vec3 ambient = 0.6 * albedo;

    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir); 
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * albedo * 0.3; 

    float specularStrength = 1.5; 
    vec3 viewDirection = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDirection + viewDirection);  
    
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0); 

    vec3 finalColor = ambient + diffuse + specular;
    
    FragColor = vec4(finalColor, waterBaseColor.a);
}