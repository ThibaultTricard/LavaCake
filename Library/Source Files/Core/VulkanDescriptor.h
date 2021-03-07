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


#ifndef VULKAN_DESCRIPTOR
#define VULKAN_DESCRIPTOR

#include "Common.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace LavaCake {

	namespace Core {
		struct CopyDescriptorInfo {
			VkDescriptorSet     TargetDescriptorSet;
			uint32_t            TargetDescriptorBinding;
			uint32_t            TargetArrayElement;
			VkDescriptorSet     SourceDescriptorSet;
			uint32_t            SourceDescriptorBinding;
			uint32_t            SourceArrayElement;
			uint32_t            DescriptorCount;
		};

		bool CreateDescriptorSetLayout(VkDevice                                          logical_device,
			std::vector<VkDescriptorSetLayoutBinding> const & bindings,
			VkDescriptorSetLayout                           & descriptor_set_layout);

		bool CreateDescriptorPool(VkDevice                                  logical_device,
			bool                                      free_individual_sets,
			uint32_t                                  max_sets_count,
			std::vector<VkDescriptorPoolSize> const & descriptor_types,
			VkDescriptorPool                        & descriptor_pool);

		bool AllocateDescriptorSets(VkDevice                                   logical_device,
			VkDescriptorPool                           descriptor_pool,
			std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
			std::vector<VkDescriptorSet>             & descriptor_sets);

		void UpdateDescriptorSets(VkDevice                                       logical_device,
			std::vector<ImageDescriptorInfo> const       & image_descriptor_infos,
			std::vector<BufferDescriptorInfo> const      & buffer_descriptor_infos,
			std::vector<TexelBufferDescriptorInfo> const & texel_buffer_descriptor_infos,
			std::vector<CopyDescriptorInfo> const        & copy_descriptor_infos);

		void BindDescriptorSets(VkCommandBuffer                      command_buffer,
			VkPipelineBindPoint                  pipeline_type,
			VkPipelineLayout                     pipeline_layout,
			uint32_t                             index_for_first_set,
			std::vector<VkDescriptorSet> const & descriptor_sets,
			std::vector<uint32_t> const        & dynamic_offsets);

		bool FreeDescriptorSets(VkDevice                       logical_device,
			VkDescriptorPool               descriptor_pool,
			std::vector<VkDescriptorSet> & descriptor_sets);

		bool ResetDescriptorPool(VkDevice          logical_device,
			VkDescriptorPool  descriptor_pool);

	}

} // namespace LavaCake

#endif // CREATING_A_SAMPLER