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