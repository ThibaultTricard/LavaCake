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

#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace LavaCake {
	namespace Core {

		void DestroyBufferView(VkDevice       logical_device,
			VkBufferView & buffer_view) {
			if (VK_NULL_HANDLE != buffer_view) {
				vkDestroyBufferView(logical_device, buffer_view, nullptr);
				buffer_view = VK_NULL_HANDLE;
			}
		}

		void DestroyBuffer(VkDevice   logical_device,
			VkBuffer & buffer) {
			if (VK_NULL_HANDLE != buffer) {
				vkDestroyBuffer(logical_device, buffer, nullptr);
				buffer = VK_NULL_HANDLE;
			}
		}


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
			VkSampler            & sampler) {
			VkSamplerCreateInfo sampler_create_info = {
				VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,    // VkStructureType          sType
				nullptr,                                  // const void             * pNext
				0,                                        // VkSamplerCreateFlags     flags
				mag_filter,                               // VkFilter                 magFilter
				min_filter,                               // VkFilter                 minFilter
				mipmap_mode,                              // VkSamplerMipmapMode      mipmapMode
				u_address_mode,                           // VkSamplerAddressMode     addressModeU
				v_address_mode,                           // VkSamplerAddressMode     addressModeV
				w_address_mode,                           // VkSamplerAddressMode     addressModeW
				lod_bias,                                 // float                    mipLodBias
				anisotropy_enable,                        // VkBool32                 anisotropyEnable
				max_anisotropy,                           // float                    maxAnisotropy
				compare_enable,                           // VkBool32                 compareEnable
				compare_operator,                         // VkCompareOp              compareOp
				min_lod,                                  // float                    minLod
				max_lod,                                  // float                    maxLod
				border_color,                             // VkBorderColor            borderColor
				unnormalized_coords                       // VkBool32                 unnormalizedCoordinates
			};

			VkResult result = vkCreateSampler(logical_device, &sampler_create_info, nullptr, &sampler);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create sampler." << std::endl;
				return false;
			}
			return true;
		}


		void DestroySampler(VkDevice    logical_device,
			VkSampler & sampler) {
			if (VK_NULL_HANDLE != sampler) {
				vkDestroySampler(logical_device, sampler, nullptr);
				sampler = VK_NULL_HANDLE;
			}
		}


		bool CreateFramebuffer(VkDevice                         logical_device,
			VkRenderPass                     render_pass,
			std::vector<VkImageView> const & attachments,
			uint32_t                         width,
			uint32_t                         height,
			uint32_t                         layers,
			VkFramebuffer                  & framebuffer) {
			VkFramebufferCreateInfo framebuffer_create_info = {
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,    // VkStructureType              sType
				nullptr,                                      // const void                 * pNext
				0,                                            // VkFramebufferCreateFlags     flags
				render_pass,                                  // VkRenderPass                 renderPass
				static_cast<uint32_t>(attachments.size()),    // uint32_t                     attachmentCount
				attachments.data(),                           // const VkImageView          * pAttachments
				width,                                        // uint32_t                     width
				height,                                       // uint32_t                     height
				layers                                        // uint32_t                     layers
			};

			VkResult result = vkCreateFramebuffer(logical_device, &framebuffer_create_info, nullptr, &framebuffer);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a framebuffer." << std::endl;
				return false;
			}
			return true;
		}

		void DestroyFramebuffer(VkDevice        logical_device,
			VkFramebuffer & framebuffer) {
			if (VK_NULL_HANDLE != framebuffer) {
				vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
				framebuffer = VK_NULL_HANDLE;
			}
		}
	}
}