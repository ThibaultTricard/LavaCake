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

#include "VulkanDescriptor.h"
#include "VulkanBuffer.h"

namespace LavaCake {

	namespace Core {
		bool CreateDescriptorSetLayout(VkDevice                                          logical_device,
			std::vector<VkDescriptorSetLayoutBinding> const & bindings,
			VkDescriptorSetLayout                           & descriptor_set_layout) {
			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // VkStructureType                      sType
				nullptr,                                              // const void                         * pNext
				0,                                                    // VkDescriptorSetLayoutCreateFlags     flags
				static_cast<uint32_t>(bindings.size()),               // uint32_t                             bindingCount
				bindings.data()                                       // const VkDescriptorSetLayoutBinding * pBindings
			};

			VkResult result = vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a layout for descriptor sets." << std::endl;
				return false;
			}
			return true;
		}

		bool CreateDescriptorPool(VkDevice                                  logical_device,
			bool                                      free_individual_sets,
			uint32_t                                  max_sets_count,
			std::vector<VkDescriptorPoolSize> const & descriptor_types,
			VkDescriptorPool                        & descriptor_pool) {
			VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
				VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,                // VkStructureType                sType
				nullptr,                                                      // const void                   * pNext
				free_individual_sets ?                                        // VkDescriptorPoolCreateFlags    flags
					VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0u,
				max_sets_count,                                               // uint32_t                       maxSets
				static_cast<uint32_t>(descriptor_types.size()),               // uint32_t                       poolSizeCount
				descriptor_types.data()                                       // const VkDescriptorPoolSize   * pPoolSizes
			};

			VkResult result = vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &descriptor_pool);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a descriptor pool." << std::endl;
				return false;
			}
			return true;
		}

		bool AllocateDescriptorSets(VkDevice                                   logical_device,
			VkDescriptorPool                           descriptor_pool,
			std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
			std::vector<VkDescriptorSet>             & descriptor_sets) {
			if (descriptor_set_layouts.size() > 0) {
				VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
					VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,         // VkStructureType                  sType
					nullptr,                                                // const void                     * pNext
					descriptor_pool,                                        // VkDescriptorPool                 descriptorPool
					static_cast<uint32_t>(descriptor_set_layouts.size()),   // uint32_t                         descriptorSetCount
					descriptor_set_layouts.data()                           // const VkDescriptorSetLayout    * pSetLayouts
				};

				descriptor_sets.resize(descriptor_set_layouts.size());

				VkResult result = vkAllocateDescriptorSets(logical_device, &descriptor_set_allocate_info, descriptor_sets.data());
				if (VK_SUCCESS != result) {
					std::cout << "Could not allocate descriptor sets." << std::endl;
					return false;
				}
				return true;
			}
			return false;
		}

		void BindDescriptorSets(VkCommandBuffer                      command_buffer,
			VkPipelineBindPoint                  pipeline_type,
			VkPipelineLayout                     pipeline_layout,
			uint32_t                             index_for_first_set,
			std::vector<VkDescriptorSet> const & descriptor_sets,
			std::vector<uint32_t> const        & dynamic_offsets) {
			vkCmdBindDescriptorSets(command_buffer, pipeline_type, pipeline_layout, index_for_first_set,
				static_cast<uint32_t>(descriptor_sets.size()), descriptor_sets.data(),
				static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
		}

		bool FreeDescriptorSets(VkDevice                       logical_device,
			VkDescriptorPool               descriptor_pool,
			std::vector<VkDescriptorSet> & descriptor_sets) {
			if (descriptor_sets.size() > 0) {
				VkResult result = vkFreeDescriptorSets(logical_device, descriptor_pool, static_cast<uint32_t>(descriptor_sets.size()), descriptor_sets.data());
				descriptor_sets.clear();
				if (VK_SUCCESS != result) {
					std::cout << "Error occurred during freeing descriptor sets." << std::endl;
					return false;
				}
			}
			return true;
		}

		bool ResetDescriptorPool(VkDevice          logical_device,
			VkDescriptorPool  descriptor_pool) {
			VkResult result = vkResetDescriptorPool(logical_device, descriptor_pool, 0);
			if (VK_SUCCESS != result) {
				std::cout << "Error occurred during descriptor pool reset." << std::endl;
				return false;
			}
			return true;
		}
	}

	

	

} // namespace LavaCake
