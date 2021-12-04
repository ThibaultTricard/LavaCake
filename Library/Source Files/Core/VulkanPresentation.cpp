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

		
	}
} // namespace LavaCake
