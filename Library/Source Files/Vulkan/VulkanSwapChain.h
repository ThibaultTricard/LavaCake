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

#ifndef VULKAN_SWAPCHAIN
#define VULKAN_SWAPCHAIN

#include "Common.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "VulkanPresentation.h"

namespace LavaCake {
	namespace Swapchain {
		bool SelectNumberOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			uint32_t                       & number_of_images);

		bool ChooseSizeOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			VkExtent2D                     & size_of_images);

		bool SelectDesiredUsageScenariosOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			VkImageUsageFlags                desired_usages,
			VkImageUsageFlags              & image_usage);

		bool SelectTransformationOfSwapchainImages(VkSurfaceCapabilitiesKHR const & surface_capabilities,
			VkSurfaceTransformFlagBitsKHR    desired_transform,
			VkSurfaceTransformFlagBitsKHR  & surface_transform);

		bool SelectFormatOfSwapchainImages(VkPhysicalDevice     physical_device,
			VkSurfaceKHR         presentation_surface,
			VkSurfaceFormatKHR   desired_surface_format,
			VkFormat           & image_format,
			VkColorSpaceKHR    & image_color_space);

		bool CreateSwapchain(VkDevice                        logical_device,
			VkSurfaceKHR                    presentation_surface,
			uint32_t                        image_count,
			VkSurfaceFormatKHR              surface_format,
			VkExtent2D                      image_size,
			VkImageUsageFlags               image_usage,
			VkSurfaceTransformFlagBitsKHR   surface_transform,
			VkPresentModeKHR                present_mode,
			VkSwapchainKHR                & old_swapchain,
			VkSwapchainKHR                & swapchain);

		bool GetHandlesOfSwapchainImages(VkDevice               logical_device,
			VkSwapchainKHR         swapchain,
			std::vector<VkImage> & swapchain_images);

		bool CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(VkPhysicalDevice       physical_device,
			VkSurfaceKHR           presentation_surface,
			VkDevice               logical_device,
			VkImageUsageFlags      swapchain_image_usage,
			VkExtent2D           & image_size,
			VkFormat             & image_format,
			VkSwapchainKHR       & old_swapchain,
			VkSwapchainKHR       & swapchain,
			std::vector<VkImage> & swapchain_images);

		bool AcquireSwapchainImage(VkDevice         logical_device,
			VkSwapchainKHR   swapchain,
			VkSemaphore      semaphore,
			VkFence          fence,
			uint32_t       & image_index);

		void DestroySwapchain(VkDevice         logical_device,
			VkSwapchainKHR & swapchain);

	}
}

#endif