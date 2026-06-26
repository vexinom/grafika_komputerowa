#version 330 core

out vec4 FragColor;

in vec3 LocalPos;
in vec2 TexCoords;

uniform vec3 sunDirection;
uniform sampler2D nightSkyTexture;
uniform sampler2D cloudTexture;
uniform float time;

uniform float cameraY;     
uniform float waterLevel;  

void main()
{
    float sunY = sunDirection.y;

    vec3 zenithDay = vec3(0.05, 0.15, 0.45);
    vec3 zenithNight = vec3(0.01, 0.01, 0.05);

    vec3 horizonDay = vec3(0.278, 0.757, 0.922);
    vec3 horizonSunset = vec3(0.9, 0.4, 0.1);
    vec3 horizonNight = vec3(0.141, 0.149, 0.22);

    float dayFactor = clamp(sunY * 2.0, 0.0, 1.0);
    vec3 currentZenith = mix(zenithNight, zenithDay, dayFactor);
    vec3 currentHorizon = mix(horizonSunset, horizonDay, clamp(sunY + 0.2, 0.0, 1.0));

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

    float gradientFactor = clamp(LocalPos.y, 0.0, 1.0);
    vec3 finalColor = mix(currentHorizon, currentZenith, gradientFactor);

    float isUnderwater = step(cameraY, waterLevel); 
    
    float depthBelowWater = max(0.0, waterLevel - cameraY);
    float horizonCutoff = depthBelowWater * 0.1; 

    if (isUnderwater > 0.0 && LocalPos.y < horizonCutoff) 
    {
        float depth = horizonCutoff - LocalPos.y; 
        vec3 deepWater = vec3(0.015, 0.11, 0.22); 
        
        float depthBlend = clamp(depth * 5.0, 0.0, 1.0);
        finalColor = mix(currentHorizon, deepWater, depthBlend);
    }

    vec3 sunDir = normalize(sunDirection);
    float sunAngle = max(dot(normalize(LocalPos), sunDir), 0.0);
    
    float sunVisibility = smoothstep(-0.2, 0.1, sunY);
    
    vec3 sunColor = mix(vec3(1.0, 0.5, 0.2), vec3(1.0, 0.95, 0.8), dayFactor);
    float sunDisc = smoothstep(0.995, 0.998, sunAngle) * sunVisibility;
    float sunGlow = pow(sunAngle, 64.0) * 0.4 * sunVisibility;

    finalColor += sunColor * (sunDisc + sunGlow);

    vec3 nightTextureColor = texture(nightSkyTexture, TexCoords).rgb;
    float nightFade = smoothstep(0.1, -0.2, sunY);
    float starHorizonFade = smoothstep(0.0, 0.15, LocalPos.y);
    finalColor = mix(finalColor, nightTextureColor, nightFade * starHorizonFade);

    vec2 cloudUV = TexCoords + vec2(time * 0.001, time * 0.005);
    vec4 cloudSample = texture(cloudTexture, cloudUV);
    float cloudAlpha = cloudSample.a * sunVisibility;
    
    float isBelowHorizon = step(LocalPos.y, horizonCutoff);   
    
    cloudAlpha *= (1.0 - (isUnderwater * isBelowHorizon));

    float zenithFade = smoothstep(0.92, 0.75, LocalPos.y);
    cloudAlpha *= zenithFade;

    float sunsetFactor = smoothstep(0.3, 0.0, abs(sunY));
    vec3 cloudTint = mix(vec3(1.0), vec3(1.0, 0.55, 0.15), sunsetFactor);
    vec3 tintedCloudColor = cloudSample.rgb * cloudTint;

    finalColor = mix(finalColor, tintedCloudColor, cloudAlpha);

    FragColor = vec4(finalColor, 1.0);
}