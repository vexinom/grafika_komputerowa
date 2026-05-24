#version 330 core

out vec4 FragColor;
in float Height;

void main()
{
    float h = (Height + 16.0) / 32.0; 
    h = clamp(h, 0.0, 1.0); 
    vec3 lowColor  = vec3(0.376, 0.090, 0.459); // #601775
    vec3 midColor  = vec3(0.141, 0.165, 0.620); // #242A9E
    vec3 highColor = vec3(0.157, 0.831, 0.820); // #28D4D1

    float t1 = smoothstep(0.0, 0.8, h);
    
    float t2 = smoothstep(0.8, 1.0, h);

    vec3 finalColor = mix(lowColor, midColor, t1);
         finalColor = mix(finalColor, highColor, t2);

    FragColor = vec4(finalColor, 1.0);
}