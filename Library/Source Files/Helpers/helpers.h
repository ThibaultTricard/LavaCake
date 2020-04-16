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

#ifndef PREPARING_A_TRANSLATION_MATRIX
#define PREPARING_A_TRANSLATION_MATRIX

#include "Tools.h"


namespace LavaCake {
	namespace Helpers {
		namespace Transformation {
			mat4 Identity();

			mat4 PrepareTranslationMatrix(float x,
				float y,
				float z);

			mat4 PrepareRotationMatrix(float           angle,
				vec3f const & axis,
				float           normalize_axis = false);

			mat4 PrepareScalingMatrix(float x,
				float y,
				float z);

			mat4 PreparePerspectiveProjectionMatrix(float aspect_ratio,
				float field_of_view,
				float near_plane,
				float far_plane);

			mat4 PrepareOrthographicProjectionMatrix(float left_plane,
				float right_plane,
				float bottom_plane,
				float top_plane,
				float near_plane,
				float far_plane);
		}

		namespace Texture {
			bool LoadTextureDataFromFile(char const                 * filename,
				int                          num_requested_components,
				std::vector<unsigned char> & image_data,
				int                        * image_width = nullptr,
				int                        * image_height = nullptr,
				int                        * image_num_components = nullptr,
				int                        * image_data_size = nullptr);
		}

		namespace Mesh {
			struct Mesh {
				std::vector<float>  Data;

				struct Part {
					uint32_t  VertexOffset;
					uint32_t  VertexCount;
				};

				std::vector<Part>   Parts;
			};

			bool Load3DModelFromObjFile(char const * filename,
				bool         load_normals,
				bool         load_texcoords,
				bool         generate_tangent_space_vectors,
				bool         unify,
				Mesh       & mesh,
				uint32_t   * vertex_stride = nullptr);
		}
	}
} // namespace LavaCake

#endif // PREPARING_A_TRANSLATION_MATRIX