#version 450

#include "push-constants.glsl"

layout(std430, set = 0, binding = 0) buffer TransformBuffer {
    mat4 transforms[];
} Transforms;

layout(location = 0) in vec3 inPosition;
layout(location = 3) in uint inTransformId;

void main() {
    mat4 transform = mat4(Transforms.transforms[inTransformId]);
    vec4 transformedPosition = transform * vec4(inPosition, 1.0);
    vec4 position = PushConstants.matrix * transformedPosition;

    float scale = pow(0.5, PushConstants.shadowMapLevel);
    position.xy *= scale;

    gl_Position = position;
}