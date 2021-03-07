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

#ifndef VULKAN_BUFFER
#define VULKAN_BUFFER

#include "Common.h"
#include "Helpers/helpers.h"

namespace LavaCake {
	
	namespace Core {


		struct BufferDescriptorInfo {
			VkDescriptorSet                     TargetDescriptorSet;
			uint32_t                            TargetDescriptorBinding;
			uint32_t                            TargetArrayElement;
			VkDescriptorType                    TargetDescriptorType;
			std::vector<VkDescriptorBufferInfo> BufferInfos;
		};

		struct TexelBufferDescriptorInfo {
			VkDescriptorSet                     TargetDescriptorSet;
			uint32_t                            TargetDescriptorBinding;
			uint32_t                            TargetArrayElement;
			VkDescriptorType                    TargetDescriptorType;
			std::vector<VkBufferView>           TexelBufferViews;
		};

		struct BufferTransition {
			VkBuffer        Buffer;
			VkAccessFlags   CurrentAccess;
			VkAccessFlags   NewAccess;
			uint32_t        CurrentQueueFamily;
			uint32_t        NewQueueFamily;
		};		

	
		bool CreateSampler(VkDevice               logical_device,
			VkFilter               mag_filter,
			VkFilter               min_filter,
			VkSamplerMipmapMode    mipmap_mode,
			VkSamplerAddressMode   u_address_mode,
			VkSamplerAddressMode   v_address_mode,
			VkSamplerAddressMode   w_address_mode,
			float                  lod_bias,
			bool                   anisotropy_enable,
			float                  max_anisotropy,
			bool                   compare_enable,
			VkCompareOp            compare_operator,
			float                  min_lod,
			float                  max_lod,
			VkBorderColor          border_color,
			bool                   unnormalized_coords,
			VkSampler            & sampler);


		bool CreateFramebuffer(VkDevice                         logical_device,
			VkRenderPass                     render_pass,
			std::vector<VkImageView> const & attachments,
			uint32_t                         width,
			uint32_t                         height,
			uint32_t                         layers,
			VkFramebuffer                  & framebuffer);

	}
}

#endif