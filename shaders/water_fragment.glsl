#version 330 core
out vec4 FragColor;

in vec3 FramePosition; 
in vec3 FrameNormal;
in vec3 WorldPos;      
in vec2 TerrainTexCoord; 

uniform vec3 cameraPos;
uniform float waterLevel; 

uniform sampler2D terrainHeightmap;
uniform float terrainYScale;
uniform float terrainYShift;

void main()
{

    float rawHeight = texture(terrainHeightmap, TerrainTexCoord).r;
    float terrainY = (rawHeight * terrainYScale) - terrainYShift;
    
    float depth = waterLevel - terrainY;
    
    if (depth < 0.0) {
        discard;
    }
    
    float foamStrength = smoothstep(2.0, 0.0, depth); 

    float colorGradient = clamp(depth * 0.02, 0.0, 1.0);
    vec4 shallowWaterColor = vec4(0.0, 0.45, 0.55, 0.2);
    vec4 deepWaterColor    = vec4(0.0, 0.02, 0.12, 1.0);
    vec3 volumeColor       = mix(shallowWaterColor.rgb, deepWaterColor.rgb, colorGradient);

    vec3 foamColor = vec3(1.0, 1.0, 1.0);
    vec3 finalVolumeColor = mix(volumeColor, foamColor, foamStrength * 0.7);

    vec3 lightColor = vec3(1.0, 1.0, 0.9);   
    vec3 norm = normalize(FrameNormal);
    
    vec3 viewDir = normalize(cameraPos - FramePosition); 
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3)); 

    vec3 ambient = 0.2 * finalVolumeColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * finalVolumeColor * lightColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = 1.5 * spec * lightColor;

    vec3 finalColor = ambient + diffuse + specular;

    FragColor = vec4(finalColor, shallowWaterColor.a + (foamStrength * 0.5));
}