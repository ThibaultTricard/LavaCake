#pragma once
#include "mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace LavaCake {
  namespace Geometry {
		// Based on:
		// Lengyel, Eric. "Computing Tangent Space Basis Vectors for an Arbitrary Mesh". Terathon Software 3D Graphics Library, 2001.
		// http://www.terathon.com/code/tangent.html

		void CalculateTangentAndBitangent(float const* normal_data,
			vec3f const& face_tangent,
			vec3f const& face_bitangent,
			float* tangent_data,
			float* bitangent_data) {
			// Gram-Schmidt orthogonalize
			vec3f const normal = vec3f({ normal_data[0], normal_data[1], normal_data[2] });
			vec3f const tangent = Normalize(face_tangent - normal * Dot(normal, face_tangent));

			// Calculate handedness
			float handedness = (Dot(Cross(normal, tangent), face_bitangent) < 0.0f) ? -1.0f : 1.0f;

			vec3f const bitangent = handedness * Cross(normal, tangent);

			tangent_data[0] = tangent[0];
			tangent_data[1] = tangent[1];
			tangent_data[2] = tangent[2];

			bitangent_data[0] = bitangent[0];
			bitangent_data[1] = bitangent[1];
			bitangent_data[2] = bitangent[2];
		}

		void GenerateTangentSpaceVectors(std::vector<float>& mesh) {
			size_t const normal_offset = 3;
			size_t const texcoord_offset = 6;
			size_t const tangent_offset = 8;
			size_t const bitangent_offset = 11;
			size_t const stride = bitangent_offset + 3;

			for (size_t i = 0; i < mesh.size(); i += stride * 3) {
					size_t i1 = i;
					size_t i2 = i1 + stride;
					size_t i3 = i2 + stride;
					vec3f const v1 = vec3f({ mesh[i1], mesh[i1 + 1], mesh[i1 + 2] });
					vec3f const v2 = vec3f({ mesh[i2], mesh[i2 + 1], mesh[i2 + 2] });
					vec3f const v3 = vec3f({ mesh[i3], mesh[i3 + 1], mesh[i3 + 2] });

					std::array<float, 2> const w1 = { mesh[i1 + texcoord_offset], mesh[i1 + texcoord_offset + 1] };
					std::array<float, 2> const w2 = { mesh[i2 + texcoord_offset], mesh[i2 + texcoord_offset + 1] };
					std::array<float, 2> const w3 = { mesh[i3 + texcoord_offset], mesh[i3 + texcoord_offset + 1] };

					float x1 = v2[0] - v1[0];
					float x2 = v3[0] - v1[0];
					float y1 = v2[1] - v1[1];
					float y2 = v3[1] - v1[1];
					float z1 = v2[2] - v1[2];
					float z2 = v3[2] - v1[2];

					float s1 = w2[0] - w1[0];
					float s2 = w3[0] - w1[0];
					float t1 = w2[1] - w1[1];
					float t2 = w3[1] - w1[1];

					float r = 1.0f / (s1 * t2 - s2 * t1);
					vec3f face_tangent = vec3f({ (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r });
					vec3f face_bitangent = vec3f({ (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r });

					CalculateTangentAndBitangent(&mesh[i1 + normal_offset], face_tangent, face_bitangent, &mesh[i1 + tangent_offset], &mesh[i1 + bitangent_offset]);
					CalculateTangentAndBitangent(&mesh[i2 + normal_offset], face_tangent, face_bitangent, &mesh[i2 + tangent_offset], &mesh[i2 + bitangent_offset]);
					CalculateTangentAndBitangent(&mesh[i3 + normal_offset], face_tangent, face_bitangent, &mesh[i3 + tangent_offset], &mesh[i3 + bitangent_offset]);
			}
		}
    
		std::pair < std::vector<float>, vertexFormat  > Load3DModelFromObjFile(std::string filename,
			bool				 load_normal,
			bool				 load_uv,
			bool         generate_tangent_space_vectors,
			bool         unify)
		{
			// Load model
			tinyobj::attrib_t                attribs;
			std::vector<tinyobj::shape_t>    shapes;
			std::vector<tinyobj::material_t> materials;
			std::string                      error;




			bool result = tinyobj::LoadObj(&attribs, &shapes, &materials, &error, filename.data());
			if (!result) {
				std::cout << "Could not open the '" << filename << "' file.";
				if (0 < error.size()) {
					std::cout << " " << error;
				}
				std::cout << std::endl;
				return {};
			}

			std::vector<primitiveFormat> description = { POS3 };
			

			if (attribs.normals.size() != 0 && load_normal) {
				description.push_back(NORM3);
			}
			if (attribs.texcoords.size() != 0 && load_uv) {
				description.push_back(UV);
			}
			if (generate_tangent_space_vectors && load_normal && load_uv) {
				description.push_back(F3);
				description.push_back(F3);
			}



			// Load model data and unify (normalize) its size and position
			float min_x = attribs.vertices[0];
			float max_x = attribs.vertices[0];
			float min_y = attribs.vertices[1];
			float max_y = attribs.vertices[1];
			float min_z = attribs.vertices[2];
			float max_z = attribs.vertices[2];

			std::vector<float> mesh;
			uint32_t offset = 0;
			for (auto& shape : shapes) {
				uint32_t part_offset = offset;

				for (auto& index : shape.mesh.indices) {
					mesh.emplace_back(attribs.vertices[3 * index.vertex_index + 0]);
					mesh.emplace_back(attribs.vertices[3 * index.vertex_index + 1]);
					mesh.emplace_back(attribs.vertices[3 * index.vertex_index + 2]);
					++offset;

					
					if (attribs.normals.size() != 0 && load_normal) {
						mesh.emplace_back(attribs.normals[3 * index.normal_index + 0]);
						mesh.emplace_back(attribs.normals[3 * index.normal_index + 1]);
						mesh.emplace_back(attribs.normals[3 * index.normal_index + 2]);
					}

					if (attribs.texcoords.size() != 0 && load_uv) {
						mesh.emplace_back(attribs.texcoords[2 * index.texcoord_index + 0]);
						mesh.emplace_back(attribs.texcoords[2 * index.texcoord_index + 1]);
					}

					if (generate_tangent_space_vectors && load_normal && load_uv) {
						// Insert temporary tangent space vectors data
						for (int i = 0; i < 6; ++i) {
							mesh.emplace_back(0.0f);
						}
					}

					if (unify) {
						if (attribs.vertices[3 * index.vertex_index + 0] < min_x) {
							min_x = attribs.vertices[3 * index.vertex_index + 0];
						}
						if (attribs.vertices[3 * index.vertex_index + 0] > max_x) {
							max_x = attribs.vertices[3 * index.vertex_index + 0];
						}
						if (attribs.vertices[3 * index.vertex_index + 1] < min_y) {
							min_y = attribs.vertices[3 * index.vertex_index + 1];
						}
						if (attribs.vertices[3 * index.vertex_index + 1] > max_y) {
							max_y = attribs.vertices[3 * index.vertex_index + 1];
						}
						if (attribs.vertices[3 * index.vertex_index + 2] < min_z) {
							min_z = attribs.vertices[3 * index.vertex_index + 2];
						}
						if (attribs.vertices[3 * index.vertex_index + 2] > max_z) {
							max_z = attribs.vertices[3 * index.vertex_index + 2];
						}
					}
				}
			}

			uint32_t stride = 3 + ((attribs.normals.size() !=0 && load_normal) ? 3 : 0) + ((attribs.texcoords.size() != 0 && load_uv) ? 2 : 0) + (generate_tangent_space_vectors ? 6 : 0);
			/*if (vertex_stride) {
				*vertex_stride = stride * sizeof(float);
			}*/

			if (generate_tangent_space_vectors) {
				GenerateTangentSpaceVectors(mesh);
			}

			if (unify) {
				float offset_x = 0.5f * (min_x + max_x);
				float offset_y = 0.5f * (min_y + max_y);
				float offset_z = 0.5f * (min_z + max_z);
				float scale_x = abs(min_x - offset_x) > abs(max_x - offset_x) ? abs(min_x - offset_x) : abs(max_x - offset_x);
				float scale_y = abs(min_y - offset_y) > abs(max_y - offset_y) ? abs(min_y - offset_y) : abs(max_y - offset_y);
				float scale_z = abs(min_z - offset_z) > abs(max_z - offset_z) ? abs(min_z - offset_z) : abs(max_z - offset_z);
				float scale = scale_x > scale_y ? scale_x : scale_y;
				scale = scale_z > scale ? 1.0f / scale_z : 1.0f / scale;

				for (size_t i = 0; i < mesh.size() - 2; i += stride) {
					mesh[i + 0] = scale * (mesh[i + 0] - offset_x);
					mesh[i + 1] = scale * (mesh[i + 1] - offset_y);
					mesh[i + 2] = scale * (mesh[i + 2] - offset_z);
				}
			}
			return { mesh,description };
    }
		

		std::pair < std::pair <std::vector<float>, std::vector<uint32_t>>, vertexFormat  > Load3DModelFromObjFile(std::string filename,
			bool				 load_normal,
			bool         unify)
		{
			// Load model
			tinyobj::attrib_t                attribs;
			std::vector<tinyobj::shape_t>    shapes;
			std::vector<tinyobj::material_t> materials;
			std::string                      error;




			bool result = tinyobj::LoadObj(&attribs, &shapes, &materials, &error, filename.data());
			if (!result) {
				std::cout << "Could not open the '" << filename << "' file.";
				if (0 < error.size()) {
					std::cout << " " << error;
				}
				std::cout << std::endl;
				return {};
			}

			std::vector<primitiveFormat> description = { POS3 };


			if (attribs.normals.size() != 0 && load_normal) {
				description.push_back(NORM3);
			}

			uint32_t stride = 3 + ((attribs.normals.size() != 0 && load_normal) ? 3 : 0);
			// Load model data and unify (normalize) its size and position
			float min_x = attribs.vertices[0];
			float max_x = attribs.vertices[0];
			float min_y = attribs.vertices[1];
			float max_y = attribs.vertices[1];
			float min_z = attribs.vertices[2];
			float max_z = attribs.vertices[2];

			//loading the position;

			std::vector<float> mesh;

			for (int i = 0; i < attribs.vertices.size(); i+=3) {
				mesh.push_back(attribs.vertices[i]);
				mesh.push_back(attribs.vertices[i+1]);
				mesh.push_back(attribs.vertices[i+2]);
				if (load_normal) {
					mesh.push_back(0);
					mesh.push_back(0);
					mesh.push_back(0);
				}
				
			}


			std::vector<uint32_t>indices;
			uint32_t offset = 0;
			for (auto& shape : shapes) {
				uint32_t part_offset = offset;

				for (auto& index : shape.mesh.indices) {
					indices.push_back(index.vertex_index);

					
					++offset;


					if (attribs.normals.size() != 0 && load_normal) {
						mesh[stride * index.vertex_index + 3] += attribs.normals[3 * index.normal_index + 0];
						mesh[stride * index.vertex_index + 4] += attribs.normals[3 * index.normal_index + 1];
						mesh[stride * index.vertex_index + 5] += attribs.normals[3 * index.normal_index + 2];
					}

					

					if (unify) {
						if (attribs.vertices[3 * index.vertex_index + 0] < min_x) {
							min_x = attribs.vertices[3 * index.vertex_index + 0];
						}
						if (attribs.vertices[3 * index.vertex_index + 0] > max_x) {
							max_x = attribs.vertices[3 * index.vertex_index + 0];
						}
						if (attribs.vertices[3 * index.vertex_index + 1] < min_y) {
							min_y = attribs.vertices[3 * index.vertex_index + 1];
						}
						if (attribs.vertices[3 * index.vertex_index + 1] > max_y) {
							max_y = attribs.vertices[3 * index.vertex_index + 1];
						}
						if (attribs.vertices[3 * index.vertex_index + 2] < min_z) {
							min_z = attribs.vertices[3 * index.vertex_index + 2];
						}
						if (attribs.vertices[3 * index.vertex_index + 2] > max_z) {
							max_z = attribs.vertices[3 * index.vertex_index + 2];
						}
					}
				}
			}



			if (unify) {
				float offset_x = 0.5f * (min_x + max_x);
				float offset_y = 0.5f * (min_y + max_y);
				float offset_z = 0.5f * (min_z + max_z);
				float scale_x = abs(min_x - offset_x) > abs(max_x - offset_x) ? abs(min_x - offset_x) : abs(max_x - offset_x);
				float scale_y = abs(min_y - offset_y) > abs(max_y - offset_y) ? abs(min_y - offset_y) : abs(max_y - offset_y);
				float scale_z = abs(min_z - offset_z) > abs(max_z - offset_z) ? abs(min_z - offset_z) : abs(max_z - offset_z);
				float scale = scale_x > scale_y ? scale_x : scale_y;
				scale = scale_z > scale ? 1.0f / scale_z : 1.0f / scale;

				for (size_t i = 0; i < mesh.size(); i += stride) {
					mesh[i + 0] = scale * (mesh[i + 0] - offset_x);
					mesh[i + 1] = scale * (mesh[i + 1] - offset_y);
					mesh[i + 2] = scale * (mesh[i + 2] - offset_z);
				}
			}

			if (load_normal) {
				//renormalize normals
				for (size_t i = 0; i < mesh.size(); i += stride) {
					float nl = (mesh[i + 3] * mesh[i + 3]);
					nl += (mesh[i + 4] * mesh[i + 4]);
					nl += (mesh[i + 5] * mesh[i + 5]);
					nl = sqrt(nl);

					if (nl > 0) {
						mesh[i + 3] = mesh[i + 3] / nl;
						mesh[i + 4] = mesh[i + 4] / nl;
						mesh[i + 5] = mesh[i + 5] / nl;
					}
				}
			}
			


			return { {mesh, indices} ,description };
		}
	
  }
}
