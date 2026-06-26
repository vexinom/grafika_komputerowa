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
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w * 0.5 + 0.5;

    if (proj.z > 1.0 || proj.z < 0.0 ||
        proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 0.0;

    float NdotL = max(dot(normal, lightDir), 0.0);
    // Tight depth range + polygon-offset acne removal -> small bias is enough.
    float bias = max(0.0015 * (1.0 - NdotL), 0.0005);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, proj.xy + vec2(x, y) * texelSize).r;
            shadow += (proj.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    return min(shadow / 9.0, 0.85);
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