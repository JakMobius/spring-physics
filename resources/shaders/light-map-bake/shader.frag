#version 450

#include "push-constants.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in float inSpecularFactor;
layout(location = 3) in vec4 inShadowMapPosition;

layout(location = 0) out vec3 outColor;
layout(set = 2, binding = 0) uniform sampler2DArrayShadow shadowMap;


void main() {
    vec3 normal = normalize(inNormal);
    float diffuse = (1.0 + dot(normal, -PushConstants.sunDirection)) * 0.5;

    vec3 rayDirection = normalize(inPosition - PushConstants.cameraPosition);
    vec3 reflected = reflect(rayDirection, normal);
    float specular = pow(max(dot(reflected, -PushConstants.sunDirection), 0.0), inSpecularFactor);

    vec3 shadowMapPosition = inShadowMapPosition.xyz / inShadowMapPosition.w;

    float shadowMapRadius = length(shadowMapPosition.xy);
    float shadowMapLevel = max(0, log2(shadowMapRadius) + 1);

    int shadowMapLayer = int(shadowMapLevel);

    float factor = pow(2, shadowMapLayer);

    shadowMapPosition.xy /= factor;
    shadowMapPosition.xy = shadowMapPosition.xy * 0.5 + 0.5;
    shadowMapPosition.z -= 0.00002 * (1 + shadowMapLayer);

    float shadow = 0;

    vec2 shadowMapScale = vec2(PushConstants.pcfScale);

    #pragma unroll
    for (int y = -1 ; y <= 1 ; y++) {

        #pragma unroll
        for (int x = -1 ; x <= 1 ; x++) {
            vec2 off = vec2(x, y) * shadowMapScale;
            shadow += texture(shadowMap, vec4(shadowMapPosition.xy + off, float(shadowMapLayer), shadowMapPosition.z)).r;
        }
    }

    shadow /= 9;

    specular *= shadow;
    diffuse = min(0.5, diffuse) + max(0, diffuse - 0.5) * shadow;

    outColor = (vec3(specular) + vec3(diffuse)) * PushConstants.sunColor;
}