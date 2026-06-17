#version 330 core
in float vType;
out vec4 FragColor;

void main()
{
    vec2 c = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(c, c);
    if (r2 > 1.0) discard;
    float edge = 1.0 - r2;

    if (vType > 0.5)
    {
        // bubble: translucent core with a bright rim
        float rim = smoothstep(0.55, 1.0, sqrt(r2));
        float a = edge * 0.30 + rim * 0.55;
        vec3 col = mix(vec3(0.55, 0.85, 0.95), vec3(1.0), rim);
        FragColor = vec4(col, a * 0.6);
    }
    else
    {
        // marine snow: soft pale dot
        float a = edge * edge * 0.5;
        FragColor = vec4(vec3(0.82, 0.90, 0.95), a);
    }
}
