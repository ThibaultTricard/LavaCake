#version 450

// Inputs from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec4 fragPosLightSpace;

// Shadow map sampler
layout(set = 1, binding = 0) uniform sampler2D shadowMap;

// Light and view information
layout(set = 0, binding = 4) uniform TransformUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
    vec3 lightPos;
    float padding;
    vec3 viewPos;
} transform;

// Output color
layout(location = 0) out vec4 outColor;

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range (Vulkan depth is already [0,1])
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Check if fragment is outside shadow map
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z > 1.0) {
        return 0.0;
    }

    // Get closest depth from shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Current fragment depth
    float currentDepth = projCoords.z;

    // Calculate bias based on surface angle to light
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

    // PCF (Percentage Closer Filtering) for softer shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(transform.lightPos - fragPos);
    vec3 viewDir = normalize(transform.viewPos - fragPos);

    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * fragColor;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * fragColor;

    // Specular
    float specularStrength = 0.5;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Calculate shadow
    float shadow = calculateShadow(fragPosLightSpace, normal, lightDir);

    // Combine lighting with shadow
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    outColor = vec4(lighting, 1.0);
}
