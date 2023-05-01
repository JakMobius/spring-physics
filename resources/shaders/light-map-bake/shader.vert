#version 450
#include "../material.glsl"
#include "push-constants.glsl"

layout(std430, set = 0, binding = 0) buffer TransformBuffer {
    mat4 transforms[];
} Transforms;

layout(std430, set = 1, binding = 0) buffer MaterialsBuffer {
    Material materials[];
} Materials;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 3) in uint inTransformId;
layout(location = 4) in uint inMaterialId;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out float outSpecularFactor;
layout(location = 3) out vec4 outShadowMapPosition;

void main() {
    mat4 transform = mat4(Transforms.transforms[inTransformId]);
    vec4 transformedPosition = transform * vec4(inPosition, 1.0);

    outNormal = mat3(transform) * inNormal;
    outPosition = vec3(transformedPosition);
    outSpecularFactor = Materials.materials[inMaterialId].specularFactor;
    outShadowMapPosition = PushConstants.shadowMapMatrix * transformedPosition;
    gl_Position = PushConstants.matrix * transformedPosition;
}