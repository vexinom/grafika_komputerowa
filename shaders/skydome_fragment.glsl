#version 330 core

out vec4 FragColor;

in vec3 LocalPos;
uniform vec3 sunDirection;

void main()
{
    float sunY = sunDirection.y;

    vec3 zenithDay = vec3(0.05, 0.15, 0.45);
    vec3 zenithNight = vec3(0.01, 0.01, 0.05);

    vec3 horizonDay = vec3(0.278, 0.757, 0.922);
    vec3 horizonSunset = vec3(0.9, 0.4, 0.1);
    vec3 horizonNight = vec3(0.086, 0.086, 0.09);

    float dayFactor = clamp(sunY * 2.0, 0.0, 1.0);
    vec3 currentZenith = mix(zenithNight, zenithDay, dayFactor);
    vec3 currentHorizon = mix(horizonSunset, horizonDay, clamp(sunY + 0.2, 0.0, 1.0));

    if(sunY < 0.0) currentHorizon = mix(horizonNight, horizonSunset, clamp(sunY + 1.0, 0.0, 1.0));


    float gradientFactor = clamp(LocalPos.y, 0.0, 1.0);
    vec3 finalColor = mix(currentHorizon, currentZenith, gradientFactor);

    vec3 sunDir = normalize(sunDirection);
    float sunAngle = max(dot(normalize(LocalPos), sunDir), 0.0);
    
    float sunVisibility = smoothstep(-0.2, 0.1, sunY);
    
    vec3 sunColor = mix(vec3(1.0, 0.5, 0.2), vec3(1.0, 0.95, 0.8), dayFactor);
    float sunDisc = smoothstep(0.995, 0.998, sunAngle) * sunVisibility;
    float sunGlow = pow(sunAngle, 64.0) * 0.4 * sunVisibility;

    finalColor += sunColor * (sunDisc + sunGlow);

    FragColor = vec4(finalColor, 1.0);
}