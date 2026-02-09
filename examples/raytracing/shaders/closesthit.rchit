#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require

// Ray payload - returns hit information to raygen
struct RayPayload {
    vec3 hitPos;       // World position of hit
    vec3 hitNormal;    // Surface normal at hit
    vec3 albedo;       // Surface color/albedo
    bool hit;          // Whether ray hit geometry
};

layout(location = 0) rayPayloadInEXT RayPayload payload;
hitAttributeEXT vec2 attribs;

// Geometry data
struct Vertex {
    vec3 pos;
    vec3 normal;
    vec3 color;
};

layout(binding = 3, set = 0, scalar) buffer Vertices { Vertex v[]; } vertices;
layout(binding = 4, set = 0) buffer Indices { uint i[]; } indices;

void main() {
    // Get triangle indices
    uint triIndex = gl_PrimitiveID * 3;
    uint i0 = indices.i[triIndex + 0];
    uint i1 = indices.i[triIndex + 1];
    uint i2 = indices.i[triIndex + 2];

    // Get vertices
    Vertex v0 = vertices.v[i0];
    Vertex v1 = vertices.v[i1];
    Vertex v2 = vertices.v[i2];

    // Compute barycentric coordinates
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    // Interpolate and return hit information
    payload.hitPos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
    payload.hitNormal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
    payload.albedo = v0.color * barycentrics.x + v1.color * barycentrics.y + v2.color * barycentrics.z;
    payload.hit = true;
}
