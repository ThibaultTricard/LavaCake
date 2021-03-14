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


#include "VulkanLoader.h"
#include "VulkanPresentation.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

namespace LavaCake {
	namespace Core {
		bool CreatePresentationSurface(VkInstance         instance,
			WindowParameters   window_parameters,
			VkSurfaceKHR     & presentation_surface) {
			VkResult result;


			result = glfwCreateWindowSurface(instance, window_parameters.Window, nullptr, &presentation_surface);


			if ((VK_SUCCESS != result) ||
				(VK_NULL_HANDLE == presentation_surface)) {
				std::cout << "Could not create presentation surface." << std::endl;
				return false;
			}
			return true;
		}

		bool SelectQueueFamilyThatSupportsPresentationToGivenSurface(VkPhysicalDevice   physical_device,
			VkSurfaceKHR       presentation_surface,
			uint32_t         & queue_family_index) {
			std::vector<VkQueueFamilyProperties> queue_families;
			if (!CheckAvailableQueueFamiliesAndTheirProperties(physical_device, queue_families)) {
				return false;
			}

			for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index) {
				VkBool32 presentation_supported = VK_FALSE;
				VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, index, presentation_surface, &presentation_supported);
				if ((VK_SUCCESS == result) &&
					(VK_TRUE == presentation_supported)) {
					queue_family_index = index;
					return true;
				}
			}
			return false;
		}

		bool SelectDesiredPresentationMode(VkPhysicalDevice   physical_device,
			VkSurfaceKHR       presentation_surface,
			VkPresentModeKHR   desired_present_mode,
			VkPresentModeKHR & present_mode) {
			// Enumerate supported present modes
			uint32_t present_modes_count = 0;
			VkResult result = VK_SUCCESS;

			result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, presentation_surface, &present_modes_count, nullptr);
			if ((VK_SUCCESS != result) ||
				(0 == present_modes_count)) {
				std::cout << "Could not get the number of supported present modes." << std::endl;
				return false;
			}

			std::vector<VkPresentModeKHR> present_modes(present_modes_count);
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, presentation_surface, &present_modes_count, present_modes.data());
			if ((VK_SUCCESS != result) ||
				(0 == present_modes_count)) {
				std::cout << "Could not enumerate present modes." << std::endl;
				return false;
			}

			// Select present mode
			for (auto & current_present_mode : present_modes) {
				if (current_present_mode == desired_present_mode) {
					present_mode = desired_present_mode;
					return true;
				}
			}

			std::cout << "Desired present mode is not supported. Selecting default FIFO mode." << std::endl;
			for (auto & current_present_mode : present_modes) {
				if (current_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
					present_mode = VK_PRESENT_MODE_FIFO_KHR;
					return true;
				}
			}

			std::cout << "VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory for all drivers!" << std::endl;
			return false;
		}

		bool GetCapabilitiesOfPresentationSurface(VkPhysicalDevice           physical_device,
			VkSurfaceKHR               presentation_surface,
			VkSurfaceCapabilitiesKHR & surface_capabilities) {
			VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, presentation_surface, &surface_capabilities);

			if (VK_SUCCESS != result) {
				std::cout << "Could not get the capabilities of a presentation surface." << std::endl;
				return false;
			}
			return true;
		}



		bool PresentImage(VkQueue                  queue,
			std::vector<VkSemaphore> rendering_semaphores,
			std::vector<PresentInfo> images_to_present) {
			VkResult result;
			std::vector<VkSwapchainKHR> swapchains;
			std::vector<uint32_t> image_indices;

			for (auto & image_to_present : images_to_present) {
				swapchains.emplace_back(image_to_present.Swapchain);
				image_indices.emplace_back(image_to_present.ImageIndex);
			}

			VkPresentInfoKHR present_info = {
				VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                   // VkStructureType          sType
				nullptr,                                              // const void*              pNext
				static_cast<uint32_t>(rendering_semaphores.size()),   // uint32_t                 waitSemaphoreCount
				rendering_semaphores.data(),                          // const VkSemaphore      * pWaitSemaphores
				static_cast<uint32_t>(swapchains.size()),             // uint32_t                 swapchainCount
				swapchains.data(),                                    // const VkSwapchainKHR   * pSwapchains
				image_indices.data(),                                 // const uint32_t         * pImageIndices
				nullptr                                               // VkResult*                pResults
			};

			result = vkQueuePresentKHR(queue, &present_info);
			switch (result) {
			case VK_SUCCESS:
				return true;
			default:
				return false;
			}
		}

		void DestroyPresentationSurface(VkInstance     instance,
			VkSurfaceKHR & presentation_surface) {
			if (presentation_surface) {
				vkDestroySurfaceKHR(instance, presentation_surface, nullptr);
				presentation_surface = VK_NULL_HANDLE;
			}
		}
	}


	
} // namespace LavaCake
