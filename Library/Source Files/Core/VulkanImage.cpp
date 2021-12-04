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
// � Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329

#include "VulkanImage.h"
#include "VulkanBuffer.h"

namespace LavaCake {
	namespace Core {
		bool CreateImage(VkDevice                logical_device,
			VkImageType             type,
			VkFormat                format,
			VkExtent3D              size,
			uint32_t                num_mipmaps,
			uint32_t                num_layers,
			VkSampleCountFlagBits   samples,
			VkImageUsageFlags       usage_scenarios,
			bool                    cubemap,
			VkImage               & image) {
			VkImageCreateInfo image_create_info = {
				VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                // VkStructureType          sType
				nullptr,                                            // const void             * pNext
				cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u, // VkImageCreateFlags       flags
				type,                                               // VkImageType              imageType
				format,                                             // VkFormat                 format
				size,                                               // VkExtent3D               extent
				num_mipmaps,                                        // uint32_t                 mipLevels
				cubemap ? 6 * num_layers : num_layers,              // uint32_t                 arrayLayers
				samples,                                            // VkSampleCountFlagBits    samples
				VK_IMAGE_TILING_OPTIMAL,                            // VkImageTiling            tiling
				usage_scenarios,                                    // VkImageUsageFlags        usage
				VK_SHARING_MODE_EXCLUSIVE,                          // VkSharingMode            sharingMode
				0,                                                  // uint32_t                 queueFamilyIndexCount
				nullptr,                                            // const uint32_t         * pQueueFamilyIndices
				VK_IMAGE_LAYOUT_UNDEFINED                           // VkImageLayout            initialLayout
			};

			VkResult result = vkCreateImage(logical_device, &image_create_info, nullptr, &image);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create an image." << std::endl;
				return false;
			}
			return true;
		}

		bool AllocateAndBindMemoryObjectToImage(VkPhysicalDevice           physical_device,
			VkDevice                   logical_device,
			VkImage                    image,
			VkMemoryPropertyFlagBits   memory_properties,
			VkDeviceMemory           & memory_object) {
			VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
			vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(logical_device, image, &memory_requirements);

			memory_object = VK_NULL_HANDLE;
			for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type) {
				if ((memory_requirements.memoryTypeBits & (1 << type)) &&
					((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties)) {

					VkMemoryAllocateInfo image_memory_allocate_info = {
						VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
						nullptr,                                  // const void       * pNext
						memory_requirements.size,                 // VkDeviceSize       allocationSize
						type                                      // uint32_t           memoryTypeIndex
					};

					VkResult result = vkAllocateMemory(logical_device, &image_memory_allocate_info, nullptr, &memory_object);
					if (VK_SUCCESS == result) {
						break;
					}
				}
			}

			if (VK_NULL_HANDLE == memory_object) {
				std::cout << "Could not allocate memory for an image." << std::endl;
				return false;
			}

			VkResult result = vkBindImageMemory(logical_device, image, memory_object, 0);
			if (VK_SUCCESS != result) {
				std::cout << "Could not bind memory object to an image." << std::endl;
				return false;
			}
			return true;
		}

		bool CreateImageView(VkDevice             logical_device,
			VkImage              image,
			VkImageViewType      view_type,
			VkFormat             format,
			VkImageAspectFlags   aspect,
			VkImageView        & image_view) {
			VkImageViewCreateInfo image_view_create_info = {
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType            sType
				nullptr,                                    // const void               * pNext
				0,                                          // VkImageViewCreateFlags     flags
				image,                                      // VkImage                    image
				view_type,                                  // VkImageViewType            viewType
				format,                                     // VkFormat                   format
				{                                           // VkComponentMapping         components
					VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         r
					VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         g
					VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         b
					VK_COMPONENT_SWIZZLE_IDENTITY               // VkComponentSwizzle         a
				},
				{                                           // VkImageSubresourceRange    subresourceRange
					aspect,                                     // VkImageAspectFlags         aspectMask
					0,                                          // uint32_t                   baseMipLevel
					VK_REMAINING_MIP_LEVELS,                    // uint32_t                   levelCount
					0,                                          // uint32_t                   baseArrayLayer
					VK_REMAINING_ARRAY_LAYERS                   // uint32_t                   layerCount
				}
			};

			VkResult result = vkCreateImageView(logical_device, &image_view_create_info, nullptr, &image_view);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create an image view." << std::endl;
				return false;
			}
			return true;
		}

		


	}
}
