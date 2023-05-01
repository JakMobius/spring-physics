#version 450
#include "../material.glsl"
#include "push-constants.glsl"

layout(std430, set = 1, binding = 0) buffer MaterialsBuffer {
    Material materials[];
} Materials;

layout(set = 2, binding = 0) uniform sampler2DMS shadowSampler;

layout(location = 0) flat in uint inMaterialId;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec4 inVertexPosition;

layout(location = 0) out vec3 outColor;

float grid(float isGrid) {

    float factorA = step(1, mod(inVertexPosition.x / 5, 4)) * 2 - 1;
    float factorB = step(1, mod(inVertexPosition.z / 5, 4)) * 2 - 1;

    float gridFactor = factorA * factorB;

    float depth = 1 / (gl_FragCoord.w * 0.1 + 1);
    gridFactor = gridFactor * depth + 0.5 * (1 - depth);

    return 1 - 0.1 * isGrid * gridFactor;
}

void main() {
    vec3 light = vec3(0);

    int numSamples = textureSamples(shadowSampler);
    for (int i = 0; i < numSamples; ++i) {
        light += texelFetch(shadowSampler, ivec2(gl_FragCoord.xy), i).rgb;
    }

    light /= float(numSamples);

    float gridFactor = grid(Materials.materials[inMaterialId].isGrid);
    outColor = Materials.materials[inMaterialId].color * (light + PushConstants.ambientLight) * gridFactor;

    float depth = gl_FragCoord.z / gl_FragCoord.w;

    float fogAmount = clamp(exp(-depth * PushConstants.fogAmount), 0.0, 1.0);

    outColor = mix(PushConstants.fogColor, outColor, vec3(fogAmount));

    outColor = pow(outColor, vec3(1.0f / 2.2f));
}