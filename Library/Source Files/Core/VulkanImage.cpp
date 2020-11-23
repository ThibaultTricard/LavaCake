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

		void SetImageMemoryBarrier(VkCommandBuffer              command_buffer,
			VkPipelineStageFlags         generating_stages,
			VkPipelineStageFlags         consuming_stages,
			std::vector<ImageTransition> image_transitions) {
			std::vector<VkImageMemoryBarrier> image_memory_barriers;

			for (auto & image_transition : image_transitions) {
				image_memory_barriers.push_back({
					VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // VkStructureType            sType
					nullptr,                                  // const void               * pNext
					image_transition.CurrentAccess,           // VkAccessFlags              srcAccessMask
					image_transition.NewAccess,               // VkAccessFlags              dstAccessMask
					image_transition.CurrentLayout,           // VkImageLayout              oldLayout
					image_transition.NewLayout,               // VkImageLayout              newLayout
					image_transition.CurrentQueueFamily,      // uint32_t                   srcQueueFamilyIndex
					image_transition.NewQueueFamily,          // uint32_t                   dstQueueFamilyIndex
					image_transition.Image,                   // VkImage                    image
					{                                         // VkImageSubresourceRange    subresourceRange
						image_transition.Aspect,                  // VkImageAspectFlags         aspectMask
						0,                                        // uint32_t                   baseMipLevel
						VK_REMAINING_MIP_LEVELS,                  // uint32_t                   levelCount
						0,                                        // uint32_t                   baseArrayLayer
						VK_REMAINING_ARRAY_LAYERS                 // uint32_t                   layerCount
					}
					});
			}

			if (image_memory_barriers.size() > 0) {
				vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(image_memory_barriers.size()), image_memory_barriers.data());
			}
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

		bool Create2DImageAndView(VkPhysicalDevice        physical_device,
			VkDevice                logical_device,
			VkFormat                format,
			VkExtent2D              size,
			uint32_t                num_mipmaps,
			uint32_t                num_layers,
			VkSampleCountFlagBits   samples,
			VkImageUsageFlags       usage,
			VkImageAspectFlags      aspect,
			VkImage               & image,
			VkDeviceMemory        & memory_object,
			VkImageView           & image_view) {
			if (!CreateImage(logical_device, VK_IMAGE_TYPE_2D, format, { size.width, size.height, 1 }, num_mipmaps, num_layers, samples, usage, false, image)) {
				return false;
			}

			if (!AllocateAndBindMemoryObjectToImage(physical_device, logical_device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object)) {
				return false;
			}

			if (!CreateImageView(logical_device, image, VK_IMAGE_VIEW_TYPE_2D, format, aspect, image_view)) {
				return false;
			}

			return true;
		}

		bool CreateLayered2DImageWithCubemapView(VkPhysicalDevice    physical_device,
			VkDevice            logical_device,
			uint32_t            size,
			uint32_t            num_mipmaps,
			VkImageUsageFlags   usage,
			VkImageAspectFlags  aspect,
			VkImage           & image,
			VkDeviceMemory    & memory_object,
			VkImageView       & image_view) {
			if (!CreateImage(logical_device, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { size, size, 1 }, num_mipmaps, 6, VK_SAMPLE_COUNT_1_BIT, usage, true, image)) {
				return false;
			}

			if (!AllocateAndBindMemoryObjectToImage(physical_device, logical_device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object)) {
				return false;
			}

			if (!CreateImageView(logical_device, image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, aspect, image_view)) {
				return false;
			}

			return true;
		}

		void DestroyImageView(VkDevice      logical_device,
			VkImageView & image_view) {
			if (VK_NULL_HANDLE != image_view) {
				vkDestroyImageView(logical_device, image_view, nullptr);
				image_view = VK_NULL_HANDLE;
			}
		}

		void DestroyImage(VkDevice   logical_device,
			VkImage  & image) {
			if (VK_NULL_HANDLE != image) {
				vkDestroyImage(logical_device, image, nullptr);
				image = VK_NULL_HANDLE;
			}
		}

		bool CreateSampledImage(VkPhysicalDevice    physical_device,
			VkDevice            logical_device,
			VkImageType         type,
			VkFormat            format,
			VkExtent3D          size,
			uint32_t            num_mipmaps,
			uint32_t            num_layers,
			VkImageUsageFlags   usage,
			bool                cubemap,
			VkImageViewType     view_type,
			VkImageAspectFlags  aspect,
			bool                linear_filtering,
			VkImage           & sampled_image,
			VkDeviceMemory    & memory_object,
			VkImageView       & sampled_image_view) {
			VkFormatProperties format_properties;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
			if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
				std::cout << "Provided format is not supported for a sampled image." << std::endl;
				return false;
			}
			if (linear_filtering &&
				!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
				std::cout << "Provided format is not supported for a linear image filtering." << std::endl;
				return false;
			}

			if (!CreateImage(logical_device, type, format, size, num_mipmaps, num_layers, VK_SAMPLE_COUNT_1_BIT, usage | VK_IMAGE_USAGE_SAMPLED_BIT, cubemap, sampled_image)) {
				return false;
			}

			if (!AllocateAndBindMemoryObjectToImage(physical_device, logical_device, sampled_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object)) {
				return false;
			}

			if (!CreateImageView(logical_device, sampled_image, view_type, format, aspect, sampled_image_view)) {
				return false;
			}
			return true;
		}

		bool CreateCombinedImageSampler(VkPhysicalDevice       physical_device,
			VkDevice               logical_device,
			VkImageType            type,
			VkFormat               format,
			VkExtent3D             size,
			uint32_t               num_mipmaps,
			uint32_t               num_layers,
			VkImageUsageFlags      usage,
			bool                   cubemap,
			VkImageViewType        view_type,
			VkImageAspectFlags     aspect,
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
			VkSampler            & sampler,
			VkImage              & sampled_image,
			VkDeviceMemory       & memory_object,
			VkImageView          & sampled_image_view) {
			if (!CreateSampler(logical_device, mag_filter, min_filter, mipmap_mode, u_address_mode, v_address_mode, w_address_mode, lod_bias, anisotropy_enable, max_anisotropy, compare_enable, compare_operator, min_lod, max_lod, border_color, unnormalized_coords, sampler)) {
				return false;
			}

			bool linear_filtering = (mag_filter == VK_FILTER_LINEAR) || (min_filter == VK_FILTER_LINEAR) || (mipmap_mode == VK_SAMPLER_MIPMAP_MODE_LINEAR);
			if (!CreateSampledImage(physical_device, logical_device, type, format, size, num_mipmaps, num_layers, usage, cubemap, view_type, aspect, linear_filtering, sampled_image, memory_object, sampled_image_view)) {
				return false;
			}
			return true;
		}

	}
}