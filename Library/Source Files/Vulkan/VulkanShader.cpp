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

#include "VulkanShader.h"

namespace LavaCake {
	namespace Shader {
		bool CreateShaderModule(VkDevice                           logical_device,
			std::vector<unsigned char> const & source_code,
			VkShaderModule                   & shader_module) {
			VkShaderModuleCreateInfo shader_module_create_info = {
				VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,              // VkStructureType              sType
				nullptr,                                                  // const void                 * pNext
				0,                                                        // VkShaderModuleCreateFlags    flags
				source_code.size(),                                       // size_t                       codeSize
				reinterpret_cast<uint32_t const *>(source_code.data())    // const uint32_t             * pCode
			};

			VkResult result = vkCreateShaderModule(logical_device, &shader_module_create_info, nullptr, &shader_module);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a shader module." << std::endl;
				return false;
			}
			return true;
		}

		void DestroyShaderModule(VkDevice         logical_device,
			VkShaderModule & shader_module) {
			if (VK_NULL_HANDLE != shader_module) {
				vkDestroyShaderModule(logical_device, shader_module, nullptr);
				shader_module = VK_NULL_HANDLE;
			}
		}
	}

}