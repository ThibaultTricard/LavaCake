// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and / or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The below copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Vulkan Cookbook
// ISBN: 9781786468154
// © Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329
//
// Chapter: 10 Helper Recipes
// Recipe:  01 Preparing a translation matrix

#include "Helpers/helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace LavaCake {
	
	namespace Helpers {
			mat4 Identity() {
				mat4 I = {
					1,0,0,0,
					0,1,0,0,
					0,0,1,0,
					0,0,0,1
				};
				return I;
			}

			mat4 PrepareTranslationMatrix(float x,
				float y,
				float z) {
				mat4 translation_matrix = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
						 x,    y,    z, 1.0f
				};
				return translation_matrix;
			}

			mat4 PrepareRotationMatrix(float           angle,
				vec3f const & axis,
				float           normalize_axis) {
				float x;
				float y;
				float z;

				if (normalize_axis) {
					vec3f normalized = Normalize(axis);
					x = normalized[0];
					y = normalized[1];
					z = normalized[2];
				}
				else {
					x = axis[0];
					y = axis[1];
					z = axis[2];
				}

				const float c = cos(Deg2Rad(angle));
				const float _1_c = 1.0f - c;
				const float s = sin(Deg2Rad(angle));

				mat4 rotation_matrix = {
					x * x * _1_c + c,
					y * x * _1_c - z * s,
					z * x * _1_c + y * s,
					0.0f,

					x * y * _1_c + z * s,
					y * y * _1_c + c,
					z * y * _1_c - x * s,
					0.0f,

					x * z * _1_c - y * s,
					y * z * _1_c + x * s,
					z * z * _1_c + c,
					0.0f,

					0.0f,
					0.0f,
					0.0f,
					1.0f
				};
				return rotation_matrix;
			}

			mat4 PrepareScalingMatrix(float x,
				float y,
				float z) {
				mat4 scaling_matrix = {
						 x, 0.0f, 0.0f, 0.0f,
					0.0f,    y, 0.0f, 0.0f,
					0.0f, 0.0f,    z, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				};
				return scaling_matrix;
			}

			mat4 PreparePerspectiveProjectionMatrix(float aspect_ratio,
				float field_of_view,
				float near_plane,
				float far_plane) {
				float f = 1.0f / tan(Deg2Rad(0.5f * field_of_view));

				mat4 perspective_projection_matrix = {
					f / aspect_ratio,
					0.0f,
					0.0f,
					0.0f,

					0.0f,
					-f,
					0.0f,
					0.0f,

					0.0f,
					0.0f,
					far_plane / (near_plane - far_plane),
					-1.0f,

					0.0f,
					0.0f,
					(near_plane * far_plane) / (near_plane - far_plane),
					0.0f
				};
				return perspective_projection_matrix;
			}

			mat4 PrepareOrthographicProjectionMatrix(float left_plane,
				float right_plane,
				float bottom_plane,
				float top_plane,
				float near_plane,
				float far_plane) {
				mat4 orthographic_projection_matrix = {
					2.0f / (right_plane - left_plane),
					0.0f,
					0.0f,
					0.0f,

					0.0f,
					2.0f / (bottom_plane - top_plane),
					0.0f,
					0.0f,

					0.0f,
					0.0f,
					1.0f / (near_plane - far_plane),
					0.0f,

					-(right_plane + left_plane) / (right_plane - left_plane),
					-(bottom_plane + top_plane) / (bottom_plane - top_plane),
					near_plane / (near_plane - far_plane),
					1.0f
				};
				return orthographic_projection_matrix;
			}

		bool LoadTextureDataFromFile(char const                 * filename,
			int                          num_requested_components,
			std::vector<unsigned char> & image_data,
			int                        * image_width,
			int                        * image_height,
			int                        * image_num_components,
			int                        * image_data_size) {
			int width = 0;
			int height = 0;
			int num_components = 0;
			std::unique_ptr<unsigned char, void(*)(void*)> stbi_data(stbi_load(filename, &width, &height, &num_components, num_requested_components), stbi_image_free);

			if ((!stbi_data) ||
				(0 >= width) ||
				(0 >= height) ||
				(0 >= num_components)) {
				std::cout << "Could not read image!" << std::endl;
				return false;
			}

			int data_size = width * height * (0 < num_requested_components ? num_requested_components : num_components);
			if (image_data_size) {
				*image_data_size = data_size;
			}
			if (image_width) {
				*image_width = width;
			}
			if (image_height) {
				*image_height = height;
			}
			if (image_num_components) {
				*image_num_components = num_components;
			}

			image_data.resize(data_size);
			std::memcpy(image_data.data(), stbi_data.get(), data_size);
			return true;
		}

	
	}
} // namespace LavaCake
