#version 450
#include "push-constants.glsl"

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;

    float depth = gl_FragCoord.z / gl_FragCoord.w;
    float fogAmount = clamp(exp(-depth * PushConstants.fogAmount), 0.0, 1.0);

    outColor.rgb = mix(PushConstants.fogColor, outColor.rgb, vec3(fogAmount));
    outColor.rgb = pow(outColor.rgb, vec3(1.0f / 2.2f));
}