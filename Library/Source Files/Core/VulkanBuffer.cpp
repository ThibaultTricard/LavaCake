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
	namespace Buffer {
		bool CreateBuffer(VkDevice             logical_device,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			VkBuffer           & buffer) {
			VkBufferCreateInfo buffer_create_info = {
				VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType
				nullptr,                                // const void           * pNext
				0,                                      // VkBufferCreateFlags    flags
				size,                                   // VkDeviceSize           size
				usage,                                  // VkBufferUsageFlags     usage
				VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode
				0,                                      // uint32_t               queueFamilyIndexCount
				nullptr                                 // const uint32_t       * pQueueFamilyIndices
			};

			VkResult result = vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &buffer);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a buffer." << std::endl;
				return false;
			}
			return true;
		}

		bool AllocateAndBindMemoryObjectToBuffer(VkPhysicalDevice           physical_device,
			VkDevice                   logical_device,
			VkBuffer                   buffer,
			VkMemoryPropertyFlagBits   memory_properties,
			VkDeviceMemory           & memory_object) {
			VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
			vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

			VkMemoryRequirements memory_requirements;
			vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);

			memory_object = VK_NULL_HANDLE;
			for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type) {
				if ((memory_requirements.memoryTypeBits & (1 << type)) &&
					((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties)) {

					VkMemoryAllocateInfo buffer_memory_allocate_info = {
						VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
						nullptr,                                  // const void       * pNext
						memory_requirements.size,                 // VkDeviceSize       allocationSize
						type                                      // uint32_t           memoryTypeIndex
					};

					VkResult result = vkAllocateMemory(logical_device, &buffer_memory_allocate_info, nullptr, &memory_object);
					if (VK_SUCCESS == result) {
						break;
					}
				}
			}

			if (VK_NULL_HANDLE == memory_object) {
				std::cout << "Could not allocate memory for a buffer." << std::endl;
				return false;
			}

			VkResult result = vkBindBufferMemory(logical_device, buffer, memory_object, 0);
			if (VK_SUCCESS != result) {
				std::cout << "Could not bind memory object to a buffer." << std::endl;
				return false;
			}
			return true;
		}

		void SetBufferMemoryBarrier(VkCommandBuffer               command_buffer,
			VkPipelineStageFlags          generating_stages,
			VkPipelineStageFlags          consuming_stages,
			std::vector<BufferTransition> buffer_transitions) {

			std::vector<VkBufferMemoryBarrier> buffer_memory_barriers;

			for (auto & buffer_transition : buffer_transitions) {
				buffer_memory_barriers.push_back({
					VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,    // VkStructureType    sType
					nullptr,                                    // const void       * pNext
					buffer_transition.CurrentAccess,            // VkAccessFlags      srcAccessMask
					buffer_transition.NewAccess,                // VkAccessFlags      dstAccessMask
					buffer_transition.CurrentQueueFamily,       // uint32_t           srcQueueFamilyIndex
					buffer_transition.NewQueueFamily,           // uint32_t           dstQueueFamilyIndex
					buffer_transition.Buffer,                   // VkBuffer           buffer
					0,                                          // VkDeviceSize       offset
					VK_WHOLE_SIZE                               // VkDeviceSize       size
					});
			}

			if (buffer_memory_barriers.size() > 0) {
				vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0, 0, nullptr, static_cast<uint32_t>(buffer_memory_barriers.size()), buffer_memory_barriers.data(), 0, nullptr);
			}
		}

		bool CreateBufferView(VkDevice       logical_device,
			VkBuffer       buffer,
			VkFormat       format,
			VkDeviceSize   memory_offset,
			VkDeviceSize   memory_range,
			VkBufferView & buffer_view) {
			VkBufferViewCreateInfo buffer_view_create_info = {
				VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,    // VkStructureType            sType
				nullptr,                                      // const void               * pNext
				0,                                            // VkBufferViewCreateFlags    flags
				buffer,                                       // VkBuffer                   buffer
				format,                                       // VkFormat                   format
				memory_offset,                                // VkDeviceSize               offset
				memory_range                                  // VkDeviceSize               range
			};

			VkResult result = vkCreateBufferView(logical_device, &buffer_view_create_info, nullptr, &buffer_view);
			if (VK_SUCCESS != result) {
				std::cout << "Could not creat buffer view." << std::endl;
				return false;
			}
			return true;
		}
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

		void BindIndexBuffer(VkCommandBuffer   command_buffer,
			VkBuffer          buffer,
			VkDeviceSize      memory_offset,
			VkIndexType       index_type) {
			vkCmdBindIndexBuffer(command_buffer, buffer, memory_offset, index_type);
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

		bool CreateUniformTexelBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkFormat             format,
			VkDeviceSize         size,
			VkImageUsageFlags    usage,
			VkBuffer           & uniform_texel_buffer,
			VkDeviceMemory     & memory_object,
			VkBufferView       & uniform_texel_buffer_view) {
			VkFormatProperties format_properties;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
			if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
				std::cout << "Provided format is not supported for a uniform texel buffer." << std::endl;
				return false;
			}

			if (!Buffer::CreateBuffer(logical_device, size, usage | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, uniform_texel_buffer)) {
				return false;
			}

			if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physical_device, logical_device, uniform_texel_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object)) {
				return false;
			}

			if (!Buffer::CreateBufferView(logical_device, uniform_texel_buffer, format, 0, VK_WHOLE_SIZE, uniform_texel_buffer_view)) {
				return false;
			}
			return true;
		}

		bool CreateStorageTexelBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkFormat             format,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			bool                 atomic_operations,
			VkBuffer           & storage_texel_buffer,
			VkDeviceMemory     & memory_object,
			VkBufferView       & storage_texel_buffer_view) {
			VkFormatProperties format_properties;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
			if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
				std::cout << "Provided format is not supported for a uniform texel buffer." << std::endl;
				return false;
			}

			if (atomic_operations &&
				!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
				std::cout << "Provided format is not supported for atomic operations on storage texel buffers." << std::endl;
				return false;
			}

			if (!Buffer::CreateBuffer(logical_device, size, usage | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, storage_texel_buffer)) {
				return false;
			}

			if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physical_device, logical_device, storage_texel_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object)) {
				return false;
			}

			if (!Buffer::CreateBufferView(logical_device, storage_texel_buffer, format, 0, VK_WHOLE_SIZE, storage_texel_buffer_view)) {
				return false;
			}
			return true;
		}

		bool CreateUniformBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			VkBuffer           & uniform_buffer,
			VkDeviceMemory     & memory_object) {
			if (!Buffer::CreateBuffer(logical_device, size, usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniform_buffer)) {
				return false;
			}

			if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physical_device, logical_device, uniform_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object)) {
				return false;
			}
			return true;
		}

		bool CreateStorageBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			VkBuffer           & storage_buffer,
			VkDeviceMemory     & memory_object) {
			if (!Buffer::CreateBuffer(logical_device, size, usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, storage_buffer)) {
				return false;
			}

			if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physical_device, logical_device, storage_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object)) {
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