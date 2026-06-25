#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in vec4 aJoints;
layout(location=4) in vec4 aWeights;

const int MAX_JOINTS = 128;
uniform mat4 jointMatrices[MAX_JOINTS];
uniform mat4 model, view, projection;

out vec3 vNormal; out vec2 vUV;

void main()
{
    mat4 skin =
        aWeights.x * jointMatrices[int(aJoints.x)] +
        aWeights.y * jointMatrices[int(aJoints.y)] +
        aWeights.z * jointMatrices[int(aJoints.z)] +
        aWeights.w * jointMatrices[int(aJoints.w)];

    vec4 skinnedPos = skin * vec4(aPos, 1.0);
    vec3 skinnedNrm = mat3(skin) * aNormal;

    vec4 world = model * skinnedPos;
    gl_Position = projection * view * world;
    vNormal = mat3(model) * skinnedNrm;
    vUV = aUV;
}
