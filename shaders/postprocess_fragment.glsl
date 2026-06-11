#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;

uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform float time;
uniform vec3 sunDirection;

const float MSCALE = 0.05; 
const float TSCALE = 0.02;  
const float SCALE  = 5.0;  
const float base_water_level = 80.0;

const mat2 mr = mat2(0.54030, 0.84147, -0.84147, 0.54030);

float hash( in float n ) { return fract(sin(n)*43758.5453); }

float noise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
        
    f = f*f*(3.0-2.0*f);    
    float n = p.x + p.y*57.0;
    
    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
    return res;
}

float fbm( in vec2 p )
{
    float f;
    f  =      0.5000*noise( p ); p = mr*p*2.02;
    f +=      0.2500*noise( p ); p = mr*p*2.33;
    f +=      0.1250*noise( p ); p = mr*p*2.01;
    f +=      0.0625*noise( p ); p = mr*p*5.21;
    return f / 0.9375; 
}

float getWaterHeight(vec2 worldXZ) 
{
    vec2 trans1 = vec2(time * 16.0, time * 23.0) * MSCALE;
    vec2 trans2 = vec2(time * -10.0, time * 14.0) * MSCALE;
    float h1 = fbm(worldXZ * TSCALE + trans1);
    float h2 = fbm(worldXZ * TSCALE * 1.5 + trans2);
    return base_water_level + (((h1 + h2) * 0.5) * SCALE);
}

vec3 ReconstructWorldPos(vec2 uv, float depth)
{
    float z = depth * 2.0 - 1.0; 
    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 worldSpacePosition = invViewProj * clipSpacePosition;
    return worldSpacePosition.xyz / worldSpacePosition.w;
}

void main()
{
    vec3 sunDir = normalize(sunDirection);
    float sunY = sunDir.y;

    vec3 deepWaterDay = vec3(0.01, 0.15, 0.3);
    vec3 deepWaterSunset = vec3(0.08, 0.04, 0.12); 
    vec3 deepWaterNight = vec3(0.005, 0.01, 0.05);

    vec3 extDay = vec3(0.015, 0.005, 0.001);
    vec3 extSunset = vec3(0.03, 0.01, 0.005); 
    vec3 extNight = vec3(0.05, 0.02, 0.01);

    vec3 currentDeepWater;
    vec3 currentExtinction;

    if (sunY > 0.0) 
    {
        float blendDay = clamp(sunY / 0.2, 0.0, 1.0);
        currentDeepWater = mix(deepWaterSunset, deepWaterDay, blendDay);
        currentExtinction = mix(extSunset, extDay, blendDay);
    } 
    else 
    {
        float blendNight = clamp((sunY + 0.2) / 0.2, 0.0, 1.0);
        currentDeepWater = mix(deepWaterNight, deepWaterSunset, blendNight);
        currentExtinction = mix(extNight, extSunset, blendNight);
    }

    vec3 color = texture(sceneColor, TexCoords).rgb;
    float depthVal = texture(sceneDepth, TexCoords).r;
    
    vec3 worldPos = ReconstructWorldPos(TexCoords, depthVal);
    vec3 rayDir = worldPos - cameraPos;
    float rayLength = length(rayDir);
    
    float camWaterLevel = getWaterHeight(cameraPos.xz);
    float targetWaterLevel = getWaterHeight(worldPos.xz);
    
    bool isCamUnderwater = cameraPos.y <= camWaterLevel;
    bool isTargetUnderwater = worldPos.y <= targetWaterLevel;
    
    float underwaterDistance = 0.0;
    
    if (isCamUnderwater && isTargetUnderwater) 
    {
        underwaterDistance = rayLength;
    }
    else if (!isCamUnderwater && isTargetUnderwater) 
    {
        float fraction = (targetWaterLevel - worldPos.y) / (cameraPos.y - worldPos.y);
        underwaterDistance = rayLength * clamp(fraction, 0.0, 1.0);
    }
    else if (isCamUnderwater && !isTargetUnderwater) 
    {
        float fraction = (camWaterLevel - cameraPos.y) / (worldPos.y - cameraPos.y);
        underwaterDistance = rayLength * clamp(fraction, 0.0, 1.0);
    }
   
    if (underwaterDistance > 0.0) 
    {
        vec3 transmission = exp(-currentExtinction * underwaterDistance);
        
        float density = 0.5; 
        if (!isCamUnderwater) {
            density *= 0.2;
        }
        
        float scatterFactor = 1.0 - exp(-underwaterDistance * density);
        
        color = (color * transmission) + (currentDeepWater * scatterFactor);
    }
    
    FragColor = vec4(color, 1.0);
}