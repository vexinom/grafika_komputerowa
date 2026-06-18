#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 chunkOffset;
uniform float time;
uniform float water_level;

out vec4 v_color;
out vec3 FragPos;
out vec3 Normal;

const float MSCALE = 0.05; 
const float TSCALE = 0.02;  
const float SCALE  = 5.0;  

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
    return ((h1 + h2) * 0.5) * SCALE;
}

void main()
{
    vec2 worldXZ = chunkOffset + aPos;

    float height = getWaterHeight(worldXZ);

    float d = 0.1; 
    float hL = getWaterHeight(worldXZ - vec2(d, 0.0));
    float hR = getWaterHeight(worldXZ + vec2(d, 0.0));
    float hD = getWaterHeight(worldXZ - vec2(0.0, d));
    float hU = getWaterHeight(worldXZ + vec2(0.0, d));
    
    Normal = normalize(vec3(hL - hR, 2.0 * d, hD - hU));

    vec4 worldPos = vec4(worldXZ.x, height + water_level, worldXZ.y, 1.0);
    FragPos = vec3(worldPos); 
    
    gl_Position = projection * view * model * worldPos;
    
    float colorIntensity = max(0.2, 1.0 - (height / SCALE) * 0.8);
    v_color = vec4(colorIntensity, colorIntensity, colorIntensity + 0.2, 1.0); 
}