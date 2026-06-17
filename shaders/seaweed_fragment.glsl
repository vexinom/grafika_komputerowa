#version 330 core
in float vH;
out vec4 FragColor;

uniform vec3 sunDirection;

void main()
{
    vec3 baseC = vec3(0.04, 0.15, 0.06);   // dark olive at the holdfast
    vec3 tipC  = vec3(0.55, 0.68, 0.14);   // yellow-green tips (as in the reference)
    vec3 col = mix(baseC, tipC, vH);

    float sunUp = clamp(sunDirection.y, 0.0, 1.0);
    float light = 0.45 + 0.55 * sunUp;     // brighter in daylight
    col *= light;
    col += tipC * 0.18 * vH;               // soft translucent glow toward the tips

    FragColor = vec4(col, 1.0);
}
