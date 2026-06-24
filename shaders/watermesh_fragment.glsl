#version 330 core

in vec4 v_color;
in vec3 FragPos; 
in vec3 Normal;
in vec4 ClipSpace;  

out vec4 FragColor;

uniform vec3 viewPos;  
uniform vec3 sunDirection; 
uniform vec3 terrainParams;
uniform float time;
uniform sampler2D heightmap;
uniform vec2 textureSize;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

void main()
{
    float sunY = sunDirection.y;
    vec3 sunDir = normalize(sunDirection);

    vec4 waterBaseColor = vec4(0.1, 0.4, 0.8, 0.6); 
    vec3 albedo = waterBaseColor.rgb * v_color.rgb * 2.5; 

    float sunIntensity = smoothstep(-0.2, 0.2, sunY);

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

    float dayFactor = clamp(sunY * 2.0, 0.0, 1.0);
    vec3 specColor = mix(vec3(1.0, 0.5, 0.2), vec3(1.0, 0.95, 0.8), dayFactor);
    vec3 specular = specularStrength * spec * specColor * sunIntensity; 

    vec3 finalColor = ambient + diffuse + specular;

    vec2 ndc = (ClipSpace.xy / ClipSpace.w) / 2.0 + 0.5;

    vec2 reflectTexCoords = vec2(ndc.x, 1.0 - ndc.y);
    vec2 refractTexCoords = vec2(ndc.x, ndc.y);

    vec2 distortion = norm.xz * 0.02;

    reflectTexCoords += distortion;
    refractTexCoords += distortion;

    reflectTexCoords = clamp(reflectTexCoords, 0.001, 0.999);
    refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);

    vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
    vec4 refractColor = texture(refractionTexture, refractTexCoords);

    float refractiveFactor = dot(viewDirection, vec3(0.0, 1.0, 0.0));
    refractiveFactor = pow(clamp(refractiveFactor, 0.0, 1.0), 1.5);

    vec4 waterFBOColor = mix(reflectColor, refractColor, refractiveFactor);

    finalColor = mix(finalColor, waterFBOColor.rgb, 0.6);

    vec2 texCoordHeight = FragPos.xz / textureSize;
    float rawY = texture(heightmap, texCoordHeight).r;
    float terrainHeight = (rawY * terrainParams.x) - terrainParams.y;
   
    float waterDepth = FragPos.y - terrainHeight;
    
    float shoreGradient = clamp(1.0 - (waterDepth / 15.0), 0.0, 1.0);
    
    /*if (waterDepth > 0.0 && waterDepth < 15.0) 
    {
        vec3 shoreColor = mix(vec3(0.5, 0.7, 1.0), vec3(1.0, 1.0, 1.0), 0.5);
        finalColor = mix(finalColor, shoreColor, shoreGradient * 0.5);
        waterBaseColor.a = mix(waterBaseColor.a, 0.9, shoreGradient);
    }*/

    float distance = length(viewPos - FragPos);
    float fogMin = 800.0;  
    float fogMax = 1600.0; 
    float fogFactor = smoothstep(fogMin, fogMax, distance);

    vec3 horizonDay = vec3(0.278, 0.757, 0.922);
    vec3 horizonSunset = vec3(0.9, 0.4, 0.1);
    vec3 horizonNight = vec3(0.086, 0.086, 0.09);

    vec3 currentHorizon;
    if (sunY > 0.0) 
    {
        float blendDay = clamp(sunY / 0.2, 0.0, 1.0);
        currentHorizon = mix(horizonSunset, horizonDay, blendDay);
    } 
    else 
    {
        float blendNight = clamp((sunY + 0.2) / 0.2, 0.0, 1.0);
        currentHorizon = mix(horizonNight, horizonSunset, blendNight);
    }

    finalColor = mix(finalColor, currentHorizon, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}