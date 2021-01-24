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
//
// Chapter: 01 Instance and Devices
// Recipe:  03 Connecting with a Vulkan Loader library

#ifndef VULKAN_LOADER
#define VULKAN_LOADER

#include "Common.h"

namespace LavaCake {
	namespace Core {
		static PFN_vkCreateAccelerationStructureKHR pfn_vkCreateAccelerationStructureKHR = 0;
		static PFN_vkDestroyAccelerationStructureKHR pfn_vkDestroyAccelerationStructureKHR = 0;
		static PFN_vkCmdBuildAccelerationStructuresKHR pfn_vkCmdBuildAccelerationStructuresKHR = 0;
		static PFN_vkCmdBuildAccelerationStructuresIndirectKHR pfn_vkCmdBuildAccelerationStructuresIndirectKHR = 0;
		static PFN_vkBuildAccelerationStructuresKHR pfn_vkBuildAccelerationStructuresKHR = 0;
		static PFN_vkCopyAccelerationStructureKHR pfn_vkCopyAccelerationStructureKHR = 0;
		static PFN_vkCopyAccelerationStructureToMemoryKHR pfn_vkCopyAccelerationStructureToMemoryKHR = 0;
		static PFN_vkCopyMemoryToAccelerationStructureKHR pfn_vkCopyMemoryToAccelerationStructureKHR = 0;
		static PFN_vkWriteAccelerationStructuresPropertiesKHR pfn_vkWriteAccelerationStructuresPropertiesKHR = 0;
		static PFN_vkCmdCopyAccelerationStructureKHR pfn_vkCmdCopyAccelerationStructureKHR = 0;
		static PFN_vkCmdCopyAccelerationStructureToMemoryKHR pfn_vkCmdCopyAccelerationStructureToMemoryKHR = 0;
		static PFN_vkCmdCopyMemoryToAccelerationStructureKHR pfn_vkCmdCopyMemoryToAccelerationStructureKHR = 0;
		static PFN_vkGetAccelerationStructureDeviceAddressKHR pfn_vkGetAccelerationStructureDeviceAddressKHR = 0;
		static PFN_vkCmdWriteAccelerationStructuresPropertiesKHR pfn_vkCmdWriteAccelerationStructuresPropertiesKHR = 0;
		static PFN_vkGetDeviceAccelerationStructureCompatibilityKHR pfn_vkGetDeviceAccelerationStructureCompatibilityKHR = 0;
		static PFN_vkGetAccelerationStructureBuildSizesKHR pfn_vkGetAccelerationStructureBuildSizesKHR = 0;
		static PFN_vkCreateAccelerationStructureNV pfn_vkCreateAccelerationStructureNV = 0;
		static PFN_vkDestroyAccelerationStructureNV pfn_vkDestroyAccelerationStructureNV = 0;
		static PFN_vkGetAccelerationStructureMemoryRequirementsNV pfn_vkGetAccelerationStructureMemoryRequirementsNV = 0;
		static PFN_vkBindAccelerationStructureMemoryNV pfn_vkBindAccelerationStructureMemoryNV = 0;
		static PFN_vkCmdBuildAccelerationStructureNV pfn_vkCmdBuildAccelerationStructureNV = 0;
		static PFN_vkCmdCopyAccelerationStructureNV pfn_vkCmdCopyAccelerationStructureNV = 0;
		static PFN_vkCmdTraceRaysNV pfn_vkCmdTraceRaysNV = 0;
		static PFN_vkCreateRayTracingPipelinesNV pfn_vkCreateRayTracingPipelinesNV = 0;
		static PFN_vkGetRayTracingShaderGroupHandlesNV pfn_vkGetRayTracingShaderGroupHandlesNV = 0;
		static PFN_vkGetAccelerationStructureHandleNV pfn_vkGetAccelerationStructureHandleNV = 0;
		static PFN_vkCmdWriteAccelerationStructuresPropertiesNV pfn_vkCmdWriteAccelerationStructuresPropertiesNV = 0;
		static PFN_vkCompileDeferredNV pfn_vkCompileDeferredNV = 0;


		bool ConnectWithVulkanLoaderLibrary(LIBRARY_TYPE & vulkan_library);
		bool LoadFunctionExportedFromVulkanLoaderLibrary(LIBRARY_TYPE const & vulkan_library);
		bool LoadGlobalLevelFunctions();
		bool LoadInstanceLevelFunctions(VkInstance                        instance,
			std::vector<char const *> const & enabled_extensions);
		bool LoadDeviceLevelFunctions(VkDevice                          logical_device,
			std::vector<char const *> const & enabled_extensions);

		bool LoadAccelerationStructureFunctions(VkDevice                          logical_device);
		void ReleaseVulkanLoaderLibrary(LIBRARY_TYPE & vulkan_library);
	}
 
	
} // namespace LavaCake

#endif // CONNECTING_WITH_A_VULKAN_LOADER_LIBRARY