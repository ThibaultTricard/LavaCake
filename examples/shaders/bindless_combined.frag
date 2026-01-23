#version 450
#extension GL_EXT_nonuniform_qualifier : require

// Bindless texture array
layout(set = 0, binding = 4) uniform sampler2D textures[];

// Input from vertex shader
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint fragTextureIndex;

// Output color
layout(location = 0) out vec4 outColor;

void main() {
    // Sample from the texture array using non-uniform index
    // The nonuniformEXT qualifier is required because the index
    // may differ between shader invocations in the same draw call
    outColor = texture(textures[nonuniformEXT(fragTextureIndex)], fragTexCoord);
}