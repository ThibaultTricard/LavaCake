#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define MAX_LIGHTS 4

// Inputs from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec4 fragPosLightSpace[MAX_LIGHTS];

// Bindless shadow map array - using descriptor indexing
layout(set = 1, binding = 0) uniform sampler2D shadowMaps[];

// Light data structure (must match vertex shader)
struct Light {
    vec4 position;      // xyz = position, w = intensity
    vec4 color;         // rgb = color, a = radius
    mat4 lightSpaceMatrix;
};

// Scene uniforms
layout(set = 0, binding = 4) uniform SceneUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPos;
    uint numLights;
    Light lights[MAX_LIGHTS];
} scene;

// Output color
layout(location = 0) out vec4 outColor;

float calculateShadow(vec4 fragPosLightSpace, uint lightIndex, vec3 normal, vec3 lightDir) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range (Vulkan depth is already [0,1])
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Check if fragment is outside shadow map (with margin for PCF)
    if (projCoords.x < 0.01 || projCoords.x > 0.99 ||
        projCoords.y < 0.01 || projCoords.y > 0.99 ||
        projCoords.z > 1.0 || projCoords.z < 0.0) {
        return 0.0;
    }

    // Current fragment depth
    float currentDepth = projCoords.z;

    // Slope-scaled bias to reduce shadow acne
    float cosTheta = max(dot(normal, lightDir), 0.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float tanTheta = sinTheta / max(cosTheta, 0.001);
    float bias = 0.002 + 0.003 * tanTheta;
    bias = clamp(bias, 0.002, 0.015);

    // PCF (Percentage Closer Filtering) for softer shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[nonuniformEXT(lightIndex)], 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMaps[nonuniformEXT(lightIndex)], projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(scene.viewPos - fragPos);

    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * fragColor;

    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    // Accumulate contribution from each light
    for (uint i = 0; i < scene.numLights; i++) {
        vec3 lightPos = scene.lights[i].position.xyz;
        float intensity = scene.lights[i].position.w;
        vec3 lightColor = scene.lights[i].color.rgb;
        float radius = scene.lights[i].color.a;

        vec3 lightDir = normalize(lightPos - fragPos);
        float distance = length(lightPos - fragPos);

        // Attenuation (inverse square with radius falloff)
        float attenuation = intensity / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        attenuation *= clamp(1.0 - distance / radius, 0.0, 1.0);

        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * fragColor * lightColor * attenuation;

        // Specular (Blinn-Phong)
        float specularStrength = 0.5;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        vec3 specular = specularStrength * spec * lightColor * attenuation;

        // Calculate shadow for this light
        float shadow = calculateShadow(fragPosLightSpace[i], i, normal, lightDir);

        // Add light contribution with shadow
        totalDiffuse += (1.0 - shadow) * diffuse;
        totalSpecular += (1.0 - shadow) * specular;
    }

    // Combine all lighting
    vec3 lighting = ambient + totalDiffuse + totalSpecular;

    outColor = vec4(lighting, 1.0);
}
