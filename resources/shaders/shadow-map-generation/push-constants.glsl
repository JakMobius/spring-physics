
layout(push_constant) uniform ShadowMappingPushConstants {
    mat4 matrix;
    uint shadowMapLevel;
} PushConstants;