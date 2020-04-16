#include "VulkanSwapchain.h"

namespace LavaCake {

	namespace Swapchain {
		bool SelectNumberOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			uint32_t                       & number_of_images) {
			number_of_images = surface_capabilities.minImageCount + 1;
			if ((surface_capabilities.maxImageCount > 0) &&
				(number_of_images > surface_capabilities.maxImageCount)) {
				number_of_images = surface_capabilities.maxImageCount;
			}
			return true;
		}

		bool ChooseSizeOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			VkExtent2D                     & size_of_images) {
			if (0xFFFFFFFF == surface_capabilities.currentExtent.width) {
				size_of_images = { 640, 480 };

				if (size_of_images.width < surface_capabilities.minImageExtent.width) {
					size_of_images.width = surface_capabilities.minImageExtent.width;
				}
				else if (size_of_images.width > surface_capabilities.maxImageExtent.width) {
					size_of_images.width = surface_capabilities.maxImageExtent.width;
				}

				if (size_of_images.height < surface_capabilities.minImageExtent.height) {
					size_of_images.height = surface_capabilities.minImageExtent.height;
				}
				else if (size_of_images.height > surface_capabilities.maxImageExtent.height) {
					size_of_images.height = surface_capabilities.maxImageExtent.height;
				}
			}
			else {
				size_of_images = surface_capabilities.currentExtent;
			}
			return true;
		}

		bool SelectDesiredUsageScenariosOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			VkImageUsageFlags                desired_usages,
			VkImageUsageFlags              & image_usage) {
			image_usage = desired_usages & surface_capabilities.supportedUsageFlags;

			return desired_usages == image_usage;
		}

		bool SelectTransformationOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			VkSurfaceTransformFlagBitsKHR    desired_transform,
			VkSurfaceTransformFlagBitsKHR  & surface_transform) {
			if (surface_capabilities.supportedTransforms & desired_transform) {
				surface_transform = desired_transform;
			}
			else {
				surface_transform = surface_capabilities.currentTransform;
			}
			return true;
		}

		bool SelectFormatOfSwapchainImages(VkPhysicalDevice     physical_device,
			VkSurfaceKHR         presentation_surface,
			VkSurfaceFormatKHR   desired_surface_format,
			VkFormat           & image_format,
			VkColorSpaceKHR    & image_color_space) {
			// Enumerate supported formats
			uint32_t formats_count = 0;
			VkResult result = VK_SUCCESS;

			result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, nullptr);
			if ((VK_SUCCESS != result) ||
				(0 == formats_count)) {
				std::cout << "Could not get the number of supported surface formats." << std::endl;
				return false;
			}

			std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, surface_formats.data());
			if ((VK_SUCCESS != result) ||
				(0 == formats_count)) {
				std::cout << "Could not enumerate supported surface formats." << std::endl;
				return false;
			}

			// Select surface format
			if ((1 == surface_formats.size()) &&
				(VK_FORMAT_UNDEFINED == surface_formats[0].format)) {
				image_format = desired_surface_format.format;
				image_color_space = desired_surface_format.colorSpace;
				return true;
			}

			for (auto & surface_format : surface_formats) {
				if ((desired_surface_format.format == surface_format.format) &&
					(desired_surface_format.colorSpace == surface_format.colorSpace)) {
					image_format = desired_surface_format.format;
					image_color_space = desired_surface_format.colorSpace;
					return true;
				}
			}

			for (auto & surface_format : surface_formats) {
				if ((desired_surface_format.format == surface_format.format)) {
					image_format = desired_surface_format.format;
					image_color_space = surface_format.colorSpace;
					std::cout << "Desired combination of format and colorspace is not supported. Selecting other colorspace." << std::endl;
					return true;
				}
			}

			image_format = surface_formats[0].format;
			image_color_space = surface_formats[0].colorSpace;
			std::cout << "Desired format is not supported. Selecting available format - colorspace combination." << std::endl;
			return true;
		}

		bool CreateSwapchain(VkDevice                        logical_device,
			VkSurfaceKHR                    presentation_surface,
			uint32_t                        image_count,
			VkSurfaceFormatKHR              surface_format,
			VkExtent2D                      image_size,
			VkImageUsageFlags               image_usage,
			VkSurfaceTransformFlagBitsKHR   surface_transform,
			VkPresentModeKHR                present_mode,
			VkSwapchainKHR                & old_swapchain,
			VkSwapchainKHR                & swapchain) {
			VkSwapchainCreateInfoKHR swapchain_create_info = {
				VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType                  sType
				nullptr,                                      // const void                     * pNext
				0,                                            // VkSwapchainCreateFlagsKHR        flags
				presentation_surface,                         // VkSurfaceKHR                     surface
				image_count,                                  // uint32_t                         minImageCount
				surface_format.format,                        // VkFormat                         imageFormat
				surface_format.colorSpace,                    // VkColorSpaceKHR                  imageColorSpace
				image_size,                                   // VkExtent2D                       imageExtent
				1,                                            // uint32_t                         imageArrayLayers
				image_usage,                                  // VkImageUsageFlags                imageUsage
				VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode                    imageSharingMode
				0,                                            // uint32_t                         queueFamilyIndexCount
				nullptr,                                      // const uint32_t                 * pQueueFamilyIndices
				surface_transform,                            // VkSurfaceTransformFlagBitsKHR    preTransform
				VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR      compositeAlpha
				present_mode,                                 // VkPresentModeKHR                 presentMode
				VK_TRUE,                                      // VkBool32                         clipped
				old_swapchain                                 // VkSwapchainKHR                   oldSwapchain
			};

			VkResult result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info, nullptr, &swapchain);
			if ((VK_SUCCESS != result) ||
				(VK_NULL_HANDLE == swapchain)) {
				std::cout << "Could not create a swapchain." << std::endl;
				return false;
			}

			if (VK_NULL_HANDLE != old_swapchain) {
				vkDestroySwapchainKHR(logical_device, old_swapchain, nullptr);
				old_swapchain = VK_NULL_HANDLE;
			}

			return true;
		}

		bool GetHandlesOfSwapchainImages(VkDevice               logical_device,
			VkSwapchainKHR         swapchain,
			std::vector<VkImage> & swapchain_images) {
			uint32_t images_count = 0;
			VkResult result = VK_SUCCESS;

			result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, nullptr);
			if ((VK_SUCCESS != result) ||
				(0 == images_count)) {
				std::cout << "Could not get the number of swapchain images." << std::endl;
				return false;
			}

			swapchain_images.resize(images_count);
			result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, swapchain_images.data());
			if ((VK_SUCCESS != result) ||
				(0 == images_count)) {
				std::cout << "Could not enumerate swapchain images." << std::endl;
				return false;
			}

			return true;
		}

		bool CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(VkPhysicalDevice       physical_device,
			VkSurfaceKHR           presentation_surface,
			VkDevice               logical_device,
			VkImageUsageFlags      swapchain_image_usage,
			VkExtent2D           & image_size,
			VkFormat             & image_format,
			VkSwapchainKHR       & old_swapchain,
			VkSwapchainKHR       & swapchain,
			std::vector<VkImage> & swapchain_images) {
			VkPresentModeKHR desired_present_mode;
			if (!Presentation::SelectDesiredPresentationMode(physical_device, presentation_surface, VK_PRESENT_MODE_MAILBOX_KHR, desired_present_mode)) {
				return false;
			}

			VkSurfaceCapabilitiesKHR surface_capabilities;
			if (!Presentation::GetCapabilitiesOfPresentationSurface(physical_device, presentation_surface, surface_capabilities)) {
				return false;
			}

			uint32_t number_of_images;
			if (!SelectNumberOfSwapchainImages(surface_capabilities, number_of_images)) {
				return false;
			}

			if (!ChooseSizeOfSwapchainImages(surface_capabilities, image_size)) {
				return false;
			}
			if ((0 == image_size.width) ||
				(0 == image_size.height)) {
				return true;
			}

			VkImageUsageFlags image_usage;
			if (!SelectDesiredUsageScenariosOfSwapchainImages(surface_capabilities, swapchain_image_usage, image_usage)) {
				return false;
			}

			VkSurfaceTransformFlagBitsKHR surface_transform;
			SelectTransformationOfSwapchainImages(surface_capabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surface_transform);

			VkColorSpaceKHR image_color_space;
			if (!SelectFormatOfSwapchainImages(physical_device, presentation_surface, { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, image_format, image_color_space)) {
				return false;
			}

			if (!CreateSwapchain(logical_device, presentation_surface, number_of_images, { image_format, image_color_space }, image_size, image_usage, surface_transform, desired_present_mode, old_swapchain, swapchain)) {
				return false;
			}

			if (!GetHandlesOfSwapchainImages(logical_device, swapchain, swapchain_images)) {
				return false;
			}
			return true;
		}

		bool AcquireSwapchainImage(VkDevice         logical_device,
			VkSwapchainKHR   swapchain,
			VkSemaphore      semaphore,
			VkFence          fence,
			uint32_t       & image_index) {
			VkResult result;

			result = vkAcquireNextImageKHR(logical_device, swapchain, 2000000000, semaphore, fence, &image_index);
			switch (result) {
			case VK_SUCCESS:
			case VK_SUBOPTIMAL_KHR:
				return true;
			default:
				return false;
			}
		}

		void DestroySwapchain(VkDevice         logical_device,
			VkSwapchainKHR & swapchain) {
			if (swapchain) {
				vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
				swapchain = VK_NULL_HANDLE;
			}
		}


	}

}