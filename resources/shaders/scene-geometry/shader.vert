#version 450

#include "push-constants.glsl"

layout(std430, set = 0, binding = 0) buffer TransformBuffer {
    mat4 transforms[];
} Transforms;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in uint inTransformId;
layout(location = 4) in uint inMaterialId;

layout(location = 0) out uint outMaterialId;
layout(location = 1) out vec2 outUv;
layout(location = 2) out vec4 outVertexPosition;

void main() {
    vec4 transformedPosition = Transforms.transforms[inTransformId] * vec4(inPosition, 1.0);
    gl_Position = PushConstants.matrix * transformedPosition;
    outMaterialId = inMaterialId;
    outUv = inUv;
    outVertexPosition = transformedPosition;
}