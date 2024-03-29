#version 450

#include "push-constants.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
    gl_Position = PushConstants.matrix * vec4(inPosition, 1.0);
    outColor = inColor;
}