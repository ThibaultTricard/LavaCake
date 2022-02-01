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


#include "Common.h"
#include "VulkanLoader.h"

namespace LavaCake {
	namespace Core {

		bool LoadFunctionExportedFromVulkanLoaderLibrary(LIBRARY_TYPE const & vulkan_library) {
#if defined _WIN32
#define LoadFunction GetProcAddress
#elif defined __linux
#define LoadFunction dlsym
#elif defined __APPLE__
#define LoadFunction dlsym
#endif

#define EXPORTED_VULKAN_FUNCTION( name )                              \
    name = (PFN_##name)LoadFunction( vulkan_library, #name );         \
    if( name == nullptr ) {                                           \
      return false;                                                   \
    }

#include "ListOfVulkanFunctions.inl"

			return true;
		}

		bool LoadGlobalLevelFunctions() {
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name )                              \
    name = (PFN_##name)vkGetInstanceProcAddr( nullptr, #name );           \
    if( name == nullptr ) {                                               \
      return false;                                                       \
    }

#include "ListOfVulkanFunctions.inl"

			return true;
		}

		bool LoadInstanceLevelFunctions(VkInstance                        instance,
			std::vector<char const *> const & enabled_extensions) {
			// Load core Vulkan API instance-level functions
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name )                                  \
    name = (PFN_##name)vkGetInstanceProcAddr( instance, #name );                \
    if( name == nullptr ) {                                                     \
      return false;                                                             \
    }

		// Load instance-level functions from enabled extensions
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )        \
    for( auto & enabled_extension : enabled_extensions ) {                      \
      if( std::string( enabled_extension ) == std::string( extension ) ) {      \
        name = (PFN_##name)vkGetInstanceProcAddr( instance, #name );            \
        if( name == nullptr ) {                                                 \
          return false;                                                         \
        }                                                                       \
      }                                                                         \
    }

#include "ListOfVulkanFunctions.inl"

			return true;
		}

		bool LoadDeviceLevelFunctions(VkDevice                          logical_device,
			std::vector<char const *> const & enabled_extensions) {
			// Load core Vulkan API device-level functions
#define DEVICE_LEVEL_VULKAN_FUNCTION( name )                                    \
    name = (PFN_##name)vkGetDeviceProcAddr( logical_device, #name );            \
    if( name == nullptr ) {                                                     \
      return false;                                                             \
    }

		// Load device-level functions from enabled extensions
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )          \
    for( auto & enabled_extension : enabled_extensions ) {                      \
      if( std::string( enabled_extension ) == std::string( extension ) ) {      \
        name = (PFN_##name)vkGetDeviceProcAddr( logical_device, #name );        \
        if( name == nullptr ) {                                                 \
        }                                                                       \
      }                                                                         \
    }

#include "ListOfVulkanFunctions.inl"

			return true;
		}
	}
  
	

	
} // namespace LavaCake
