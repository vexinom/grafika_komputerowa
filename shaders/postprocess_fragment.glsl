#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;

uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform float time;
uniform vec3 sunDirection;
uniform float base_water_level = 80.0;


// controls
uniform float fogDensity;     // user-tunable underwater visibility (lower = clearer)
uniform vec2  sunScreenPos;   // sun position in screen UV (for a soft lens glow)
uniform float sunVisible;     // 1.0 when the sun is in front of the camera

const float SCALE = 5.0;

const mat2 mr = mat2(0.54030, 0.84147, -0.84147, 0.54030);

float hash(in float n) { return fract(sin(n) * 43758.5453); }

float noise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 57.0;
    return mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
               mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y);
}

float fbm(in vec2 p)
{
    float f = 0.5000 * noise(p); p = mr * p * 2.02;
    f += 0.2500 * noise(p);      p = mr * p * 2.33;
    f += 0.1250 * noise(p);      p = mr * p * 2.01;
    f += 0.0625 * noise(p);
    return f / 0.9375;
}

float getWaterHeight(vec2 worldXZ)
{
    vec2 t1 = vec2(time * 16.0, time * 23.0) * 0.05;
    vec2 t2 = vec2(time * -10.0, time * 14.0) * 0.05;
    float h1 = fbm(worldXZ * 0.02 + t1);
    float h2 = fbm(worldXZ * 0.03 + t2);
    return base_water_level + (((h1 + h2) * 0.5) * SCALE);
}

vec3 ReconstructWorldPos(vec2 uv, float depth)
{
    float z = depth * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 world = invViewProj * clip;
    return world.xyz / world.w;
}

// animated caustic pattern (cheap voronoi ripples) on world XZ
float caustics(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    float m = 1.0;
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++)
    {
        vec2 g = vec2(float(x), float(y));
        vec2 o = vec2(hash(dot(i + g, vec2(7.0, 113.0))), hash(dot(i + g, vec2(269.0, 31.0))));
        o = 0.5 + 0.5 * sin(time * 1.6 + 6.2831 * o);
        vec2 r = g + o - f;
        m = min(m, dot(r, r));
    }
    float c = 1.0 - smoothstep(0.0, 0.6, m);
    return c * c;
}

void main()
{
    vec3 sunDir = normalize(sunDirection);
    float sunY = sunDir.y;

    vec3 deepWater  = vec3(0.015, 0.11, 0.22);  // darker teal so the deep reads as deep (and rays contrast)
    vec3 extinction = vec3(0.0065, 0.0030, 0.0016) * fogDensity;

    vec3 color = texture(sceneColor, TexCoords).rgb;
    float depthVal = texture(sceneDepth, TexCoords).r;
    bool isSky = depthVal >= 0.9999;

    vec3 worldPos = ReconstructWorldPos(TexCoords, depthVal);
    vec3 rd = normalize(worldPos - cameraPos);
    float rayLength = length(worldPos - cameraPos);

    float camWaterLevel = getWaterHeight(cameraPos.xz);
    float targetWaterLevel = getWaterHeight(worldPos.xz);
    bool isCamUnderwater = cameraPos.y <= camWaterLevel;
    bool isTargetUnderwater = worldPos.y <= targetWaterLevel;

    // ---- caustics on submerged geometry ----
    if (isTargetUnderwater && !isSky)
    {
        float depthBelow = targetWaterLevel - worldPos.y;
        float shallow = clamp(1.0 - depthBelow / 150.0, 0.0, 1.0);
        float caus = caustics(worldPos.xz * 0.06) + 0.5 * caustics(worldPos.xz * 0.13 + 7.3);
        color += vec3(0.45, 0.85, 0.95) * caus * shallow * 0.55 * max(sunY, 0.1);
    }

    // ---- underwater fog / colour attenuation ----
    float underwaterDistance = 0.0;
    if (isCamUnderwater && isTargetUnderwater)
        underwaterDistance = rayLength;
    else if (!isCamUnderwater && isTargetUnderwater)
        underwaterDistance = rayLength * clamp((targetWaterLevel - worldPos.y) / max(cameraPos.y - worldPos.y, 0.001), 0.0, 1.0);
    else if (isCamUnderwater && !isTargetUnderwater)
        underwaterDistance = rayLength * clamp((camWaterLevel - cameraPos.y) / max(worldPos.y - cameraPos.y, 0.001), 0.0, 1.0);

    if (underwaterDistance > 0.0)
    {
        vec3 transmission = exp(-extinction * underwaterDistance);
        float density = 0.015 * fogDensity;
        float scatter = 1.0 - exp(-underwaterDistance * density);
        if (isSky) scatter = 1.0;
        color = color * transmission + deepWater * scatter;
    }

    // ---- volumetric god rays: march the view ray through the water and add
    //      parallel light shafts, so they are visible throughout the volume
    //      (not only where geometry occludes the sun). Tuned to stay subtle. ----
    if (isCamUnderwater && sunY > 0.0)
    {
        const int STEPS = 24;
        float far = min(rayLength, 1400.0);
        vec3 stepv = rd * (far / float(STEPS));
        float dith = fract(sin(dot(TexCoords, vec2(12.9898, 78.233))) * 43758.5453);
        vec3 p = cameraPos + stepv * dith;

        vec2 sh = normalize(sunDir.xz + vec2(1e-4));   // sun's horizontal direction
        vec2 perp = vec2(-sh.y, sh.x);                 // axis across the shafts

        float acc = 0.0;
        for (int i = 0; i < STEPS; i++)
        {
            p += stepv;
            if (p.y > base_water_level) continue;
            float reach = exp(-(base_water_level - p.y) * 0.0045);   // shafts fade with depth
            float coord = dot(p.xz, perp) * 0.02;
            float a = 0.5 + 0.5 * sin(coord + time * 0.5);
            float b = 0.5 + 0.5 * sin(coord * 2.3 - time * 0.3);
            float shaft = pow(a, 6.0) * 0.7 + pow(b, 9.0) * 0.3;     // narrow bright beams
            acc += shaft * reach;
        }
        acc /= float(STEPS);
        float toSun = 0.55 + 0.45 * max(dot(rd, sunDir), 0.0);       // brighter looking toward the sun
        color += vec3(0.40, 0.70, 0.92) * acc * toSun * sunY * 1.6;
    }

    // ---- bloom: bright-pass blur of the scene, added back as glow ----
    {
        vec2 ts = 1.0 / vec2(textureSize(sceneColor, 0));
        vec3 bloom = vec3(0.0);
        float wsum = 0.0;
        for (int r = 1; r <= 2; r++)
        {
            float radius = float(r) * 3.0;
            for (int s = 0; s < 8; s++)
            {
                float a = 6.2831 * float(s) / 8.0;
                vec2 off = vec2(cos(a), sin(a)) * radius * ts;
                vec3 c = texture(sceneColor, TexCoords + off).rgb;
                vec3 bright = max(c - 0.7, 0.0);
                float w = 1.0 / float(r);
                bloom += bright * w;
                wsum += w;
            }
        }
        bloom /= max(wsum, 0.001);
        color += bloom * 0.45;
    }

    // ---- soft lens glow around the sun disc ----
    if (sunVisible > 0.5 && sunY > 0.0)
    {
        float g = 1.0 - smoothstep(0.0, 0.40, distance(TexCoords, sunScreenPos));
        color += vec3(0.5, 0.8, 1.0) * g * g * 0.12 * sunY;
    }

    // gentle vignette (scene shaders already output sRGB, so no extra gamma here)
    float vig = smoothstep(1.25, 0.45, distance(TexCoords, vec2(0.5)));
    color *= mix(0.85, 1.0, vig);

    FragColor = vec4(color, 1.0);
}
