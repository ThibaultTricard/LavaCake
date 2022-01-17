#include "Device.h"

namespace LavaCake {
  namespace Framework {
  
#ifdef _WIN32
  const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
  };
#else
  const std::vector<const char*> validationLayers = {};
#endif
  
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
      "LavaCake",                                       // const char              * pEngineName
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
  
  
  bool CreateLogicalDevice(VkPhysicalDevice                  physical_device,
                           std::vector<Core::QueueInfo>            queue_infos,
                           std::vector<char const *> const & desired_extensions,
                           VkPhysicalDeviceFeatures        * desired_features,
                           void*                              pNextChain,
                           VkDevice                        & logical_device) {
    std::vector<VkExtensionProperties> available_extensions;
    
    
    uint32_t extensions_count = 0;
    VkResult result = VK_SUCCESS;
    
    result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, nullptr);
    if ((result != VK_SUCCESS) ||
        (extensions_count == 0)) {
      std::cout << "Could not get the number of device extensions." << std::endl;
      return false;
    }
    
    available_extensions.resize(extensions_count);
    result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, available_extensions.data());
    if ((result != VK_SUCCESS) ||
        (extensions_count == 0)) {
      std::cout << "Could not enumerate device extensions." << std::endl;
      return false;
    }
    
    for (auto & extension : desired_extensions) {
      if (!IsExtensionSupported(available_extensions, extension)) {
        std::cout << "Extension named '" << extension << "' is not supported by a physical device." << std::endl;
        return false;
      }
    }
    
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    
    for (auto & info : queue_infos) {
      queue_create_infos.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,       // VkStructureType                  sType
        nullptr,                                          // const void                     * pNext
        0,                                                // VkDeviceQueueCreateFlags         flags
        info.FamilyIndex,                                 // uint32_t                         queueFamilyIndex
        static_cast<uint32_t>(info.Priorities.size()),    // uint32_t                         queueCount
        info.Priorities.data()                            // const float                    * pQueuePriorities
      });
    };
    
    VkDeviceCreateInfo device_create_info = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,               // VkStructureType                  sType
      nullptr,                                            // const void                     * pNext
      0,                                                  // VkDeviceCreateFlags              flags
      static_cast<uint32_t>(queue_create_infos.size()),   // uint32_t                         queueCreateInfoCount
      queue_create_infos.data(),                          // const VkDeviceQueueCreateInfo  * pQueueCreateInfos
      0,                                                  // uint32_t                         enabledLayerCount
      nullptr,                                            // const char * const             * ppEnabledLayerNames
      static_cast<uint32_t>(desired_extensions.size()),   // uint32_t                         enabledExtensionCount
      desired_extensions.data(),                          // const char * const             * ppEnabledExtensionNames
      desired_features                                    // const VkPhysicalDeviceFeatures * pEnabledFeatures
    };
    
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    if (pNextChain) {
      physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
      physicalDeviceFeatures2.features = *desired_features;
      physicalDeviceFeatures2.pNext = pNextChain;
      device_create_info.pEnabledFeatures = nullptr;
      device_create_info.pNext = &physicalDeviceFeatures2;
    }
    
    result = vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device);
    if ((result != VK_SUCCESS) ||
        (logical_device == VK_NULL_HANDLE)) {
      std::cout << "Could not create logical device." << std::endl;
      return false;
    }
    
    return true;
  }
  
  
  
  void DestroyLogicalDevice(VkDevice & logical_device) {
    if (logical_device) {
      
    }
  }
  
		Device* Device::m_device;

		VkPhysicalDevice& Device::getPhysicalDevice() {
			return m_physical;
		}

		VkDevice& Device::getLogicalDevice() {
			return m_logical;
		}

		PresentationQueue* Device::getPresentQueue() {
			return m_presentQueue;
		};

		GraphicQueue* Device::getGraphicQueue(int i) {
			return &m_graphicQueues[i];
		};

		ComputeQueue* Device::getComputeQueue(int i) {
			return& m_computeQueues[i];
		}


		VkCommandPool  Device::getCommandPool() {
			return m_commandPool;
		};

		VkSurfaceKHR  Device::getSurface() {
			return m_presentationSurface;
		};

    void Device::enableRaytracing() {
      m_raytracingEnabled = true;
    }

    void Device::enableMeshShader() {
      m_meshShaderEnabled = true;
    }

		void Device::initDevices(int nbComputeQueue, int nbGraphicQueue, WindowParameters&	WindowParams, VkPhysicalDeviceFeatures* desired_device_features) {

      
      //============== Loading Vulkan
#if defined _WIN32
      m_vulkanLibrary = LoadLibrary("vulkan-1.dll");
#elif defined __linux
      m_vulkanLibrary = dlopen("libvulkan.so.1", RTLD_NOW);
#elif defined __APPLE__
      m_vulkanLibrary = dlopen("libvulkan.1.dylib", RTLD_NOW);
#endif
      
      if (m_vulkanLibrary == nullptr) {
        ErrorCheck::setError((char*)"Could not connect with a Vulkan Runtime library.");
        return;
      }

			if (!LavaCake::Core::LoadFunctionExportedFromVulkanLoaderLibrary(m_vulkanLibrary)) {
				ErrorCheck::setError((char*)"Could not load Vulkan library while initializing the device");
			}

			if (!LavaCake::Core::LoadGlobalLevelFunctions()) {
				ErrorCheck::setError((char*)"Could not load global level Vulkan functions while initializing the device");
			}



			

			std::vector<char const*> instance_extensions;
			instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
      instance_extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
      instance_extensions.emplace_back(
#ifdef VK_USE_PLATFORM_WIN32_KHR
                                      VK_KHR_WIN32_SURFACE_EXTENSION_NAME
                                      
#elif defined VK_USE_PLATFORM_XCB_KHR
                                      VK_KHR_XCB_SURFACE_EXTENSION_NAME
                                      
#elif defined VK_USE_PLATFORM_XLIB_KHR
                                      VK_KHR_XLIB_SURFACE_EXTENSION_NAME
                                      
#elif defined VK_USE_PLATFORM_MACOS_MVK
                                      VK_MVK_MACOS_SURFACE_EXTENSION_NAME
                                      );
      instance_extensions.emplace_back(
                                      VK_EXT_METAL_SURFACE_EXTENSION_NAME
#endif
                                      );
      
			if (!CreateVulkanInstance(instance_extensions, "LavaCake", m_instance)) {
				ErrorCheck::setError((char*)"Could not load Vulkan while initializing the device");
			}

			if (!LavaCake::Core::LoadInstanceLevelFunctions(m_instance, instance_extensions)) {
				ErrorCheck::setError((char*)"Could not load instance level Vulkan functions while initializing the device");
			}

      VkResult result = glfwCreateWindowSurface(m_instance, WindowParams.Window, nullptr, &m_presentationSurface);
			if (result != VK_SUCCESS || m_presentationSurface == VK_NULL_HANDLE) {
				ErrorCheck::setError((char*)"Failed to create presentation surface");
			}

      //=========================== getting all physical Devices
			std::vector<VkPhysicalDevice> physical_devices;

      uint32_t devices_count = 0;
      result = VK_SUCCESS;
      
      result = vkEnumeratePhysicalDevices(m_instance, &devices_count, nullptr);
      if ((result != VK_SUCCESS) ||
          (devices_count == 0)) {
        ErrorCheck::setError((char*)"Could not get the number of available physical devices.");
        return;
      }
      
      physical_devices.resize(devices_count);
      result = vkEnumeratePhysicalDevices(m_instance, &devices_count, physical_devices.data());
      if ((result != VK_SUCCESS) ||
          (devices_count == 0)) {
        ErrorCheck::setError((char*)"Could not enumerate physical devices.");
        return;
      }
      
			for (int i = 0; i < nbComputeQueue; i++) {
				m_computeQueues.push_back(ComputeQueue());
			}

			for (int i = 0; i < nbGraphicQueue; i++) {
				m_graphicQueues.push_back(GraphicQueue());
			}

			for (auto& physical_device : physical_devices) {
                VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
                VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
                VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
				std::vector<char const*> device_extensions;
				std::vector <LavaCake::Core::QueueInfo> requested_queues;
				for (int i = 0; i < nbGraphicQueue; i++) {
					if (!m_graphicQueues[i].initIndex(&physical_device)) {
						goto endloop;
					}
				}

				for (int i = 0; i < nbComputeQueue; i++) {
					if (!m_computeQueues[i].initIndex(&physical_device)) {
						goto endloop;
					}
				}

				if (!m_presentQueue->initIndex(&physical_device, &m_presentationSurface)) {
					continue;
				}

				requested_queues.push_back({ m_graphicQueues[0].getIndex(), { 1.0f } });
				for (int i = 1; i < nbGraphicQueue; i++) {
					for (int j = 0; j < i; j++) {
						if (m_graphicQueues[i].getIndex() == m_graphicQueues[j].getIndex()) {
							goto endConcGrapics;
						}
					}
					requested_queues.push_back({ m_graphicQueues[i].getIndex(),{ 1.0f } });
				endConcGrapics:;
				}

				for (int i = 1; i < nbComputeQueue; i++) {
					for (int j = 0; j < nbGraphicQueue; j++) {
						if (m_computeQueues[i].getIndex() == m_graphicQueues[j].getIndex()) {
							goto 	endConcCompute;
						}
					}

					for (int j = 0; j < i; j++) {
						if (m_graphicQueues[i].getIndex() == m_computeQueues[j].getIndex()) {
							break;
						}
					}
					requested_queues.push_back({ m_computeQueues[i].getIndex(),{ 1.0f } });
				endConcCompute:;
				}


				for (int j = 0; j < nbGraphicQueue; j++) {
					if (m_presentQueue->getIndex() == m_graphicQueues[j].getIndex()) {
						goto 	endConcPresent;
					}
				}

				for (int j = 0; j < nbComputeQueue; j++) {
					if (m_presentQueue->getIndex() == m_computeQueues[j].getIndex()) {
						break;
					}
				}
				requested_queues.push_back({ m_presentQueue->getIndex(),{ 1.0f } });
			endConcPresent:;


        if (m_raytracingEnabled) {
          device_extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
          device_extensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
          device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
          device_extensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
          device_extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
          device_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
          device_extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
          device_extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);



          enabledBufferDeviceAddresFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
          enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;

          enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
          enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
          enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddresFeatures;


          enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
          enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
          enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;
        }

        if (0) { // ray query
          //device_extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
        }

        if (m_meshShaderEnabled) {

        }


				device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

				if (desired_device_features == nullptr) {
					desired_device_features = new VkPhysicalDeviceFeatures();
				}
				if (!CreateLogicalDevice(physical_device, requested_queues, device_extensions, desired_device_features, (void*)(&enabledAccelerationStructureFeatures), m_logical)) {
					continue;
				}
				else {
					m_physical = physical_device;
					LavaCake::Core::LoadDeviceLevelFunctions(m_logical, device_extensions);
					
					//Todo Check if getHandle()  works
					for (int i = 0; i < nbGraphicQueue; i++) {
						LavaCake::vkGetDeviceQueue(m_logical, m_graphicQueues[i].getIndex(), 0, &m_graphicQueues[i].getHandle());
					}

					for (int i = 0; i < nbComputeQueue; i++) {
						LavaCake::vkGetDeviceQueue(m_logical, m_computeQueues[i].getIndex(), 0, &m_computeQueues[i].getHandle());
					}

					LavaCake::vkGetDeviceQueue(m_logical, m_presentQueue->getIndex(), 0, &m_presentQueue->getHandle());
					break;
				}

			endloop:;
			}

			if (!m_logical) {
				ErrorCheck::setError((char*)"The logical device could not be created");
			}

      
      VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,   // VkStructureType              sType
        nullptr,                                      // const void                 * pNext
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,                                   // VkCommandPoolCreateFlags     flags
        m_graphicQueues[0].getIndex()                                  // uint32_t                     queueFamilyIndex
      };
      
      result = vkCreateCommandPool(m_logical, &command_pool_create_info, nullptr, &m_commandPool);
      if (result != VK_SUCCESS) {
        ErrorCheck::setError((char*)"Could not create command pool.");
        return;
      }
      
		}

		
		void Device::waitForAllCommands() {
			if (m_logical) {
				VkResult result = vkDeviceWaitIdle(m_logical);
				if (result != VK_SUCCESS) {
					ErrorCheck::setError((char*)"Waiting on a device failed.");
				}
			}
		}


		void Device::end() {
			waitForAllCommands();
      delete m_device;
		}
  }
}
