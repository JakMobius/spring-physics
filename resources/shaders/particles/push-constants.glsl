
layout(push_constant) uniform BasicPushConstants {
    mat4 matrix;
    vec3 fogColor;
    float fogAmount;
} PushConstants;