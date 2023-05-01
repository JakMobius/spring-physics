
layout(push_constant) uniform ShadingPushConstants {
    mat4 matrix;
    mat4 shadowMapMatrix;
    vec3 cameraPosition;
    float pcfScale;
    vec2 viewportSize;
    vec3 sunDirection;
    vec3 sunColor;
} PushConstants;