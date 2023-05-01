
layout(push_constant) uniform BasicPushConstants {
    mat4 matrix;
    vec3 ambientLight;
    vec2 viewportSize;
    vec3 fogColor;
    float fogAmount;
} PushConstants;