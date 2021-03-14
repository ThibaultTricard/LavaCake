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

#include "VulkanInstance.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"



namespace LavaCake {
	namespace Core {


		bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> & available_extensions) {
			uint32_t extensions_count = 0;
			VkResult result = VK_SUCCESS;

			result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
			if ((result != VK_SUCCESS) ||
				(extensions_count == 0)) {
				std::cout << "Could not get the number of instance extensions." << std::endl;
				return false;
			}

			available_extensions.resize(extensions_count);
			result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, available_extensions.data());
			if ((result != VK_SUCCESS) ||
				(extensions_count == 0)) {
				std::cout << "Could not enumerate instance extensions." << std::endl;
				return false;
			}

			return true;
		}

		bool CreateVulkanInstance(std::vector<char const *> const & desired_extensions,
			char const * const                application_name,
			VkInstance                      & instance) {
			std::vector<VkExtensionProperties> available_extensions;
			if (!CheckAvailableInstanceExtensions(available_extensions)) {
				return false;
			}

			for (auto & extension : desired_extensions) {
				if (!IsExtensionSupported(available_extensions, extension)) {
					std::cout << "Extension named '" << extension << "' is not supported by an Instance object." << std::endl;
					return false;
				}
			}

			VkApplicationInfo application_info = {
				VK_STRUCTURE_TYPE_APPLICATION_INFO,                 // VkStructureType           sType
				nullptr,                                            // const void              * pNext
				application_name,                                   // const char              * pApplicationName
				VK_MAKE_VERSION(1, 0, 0),                         // uint32_t                  applicationVersion
				"LavaCake",																				// const char              * pEngineName
				VK_MAKE_VERSION(1, 2, 0),                         // uint32_t                  engineVersion
				VK_MAKE_VERSION(1, 2, 0)                          // uint32_t                  apiVersion
			};

			VkInstanceCreateInfo instance_create_info = {
				VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             // VkStructureType           sType
				nullptr,                                            // const void              * pNext
				0,                                                  // VkInstanceCreateFlags     flags
				&application_info,                                  // const VkApplicationInfo * pApplicationInfo
				static_cast<uint32_t>(validationLayers.size()),     // uint32_t                  enabledLayerCount
				validationLayers.data(),                            // const char * const      * ppEnabledLayerNames
				static_cast<uint32_t>(desired_extensions.size()),   // uint32_t                  enabledExtensionCount
				desired_extensions.data()                           // const char * const      * ppEnabledExtensionNames
			};

			VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
			if ((result != VK_SUCCESS) ||
				(instance == VK_NULL_HANDLE)) {
				std::cout << "Could not create Vulkan instance." << std::endl;
				return false;
			}
			
			return true;
		}


		void DestroyVulkanInstance(VkInstance & instance) {
			if (instance) {
				vkDestroyInstance(instance, nullptr);
				instance = VK_NULL_HANDLE;
			}
		}

		bool CreateVulkanInstanceWithWsiExtensionsEnabled(std::vector<char const *> & desired_extensions,
			char const * const          application_name,
			VkInstance                & instance) {
			desired_extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
			desired_extensions.emplace_back(
#ifdef VK_USE_PLATFORM_WIN32_KHR
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#elif defined VK_USE_PLATFORM_XCB_KHR
				VK_KHR_XCB_SURFACE_EXTENSION_NAME

#elif defined VK_USE_PLATFORM_XLIB_KHR
				VK_KHR_XLIB_SURFACE_EXTENSION_NAME
                                            
#elif defined VK_USE_PLATFORM_MACOS_MVK
              VK_MVK_MACOS_SURFACE_EXTENSION_NAME
            );
            desired_extensions.emplace_back(
              VK_EXT_METAL_SURFACE_EXTENSION_NAME
#endif
			);

			return CreateVulkanInstance(desired_extensions, application_name, instance);
		}


	}
}
