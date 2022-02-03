#include "Device.h"
#include <algorithm>
#ifdef _WIN32
#include <Windows.h>
#elif defined __linux
#include <dlfcn.h>
#elif defined __APPLE__
#include <dlfcn.h>
#endif

namespace LavaCake {
  namespace Framework {
  
#ifdef _WIN32
  const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
  };
#else
  const std::vector<const char*> validationLayers = {};
#endif

  const std::vector<const char*> raytracingExtension = {
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
  };

  const std::vector<const char*> meshShaderExtension = {
    VK_NV_MESH_SHADER_EXTENSION_NAME
  };

  
  bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> & available_extensions) {
    uint32_t extensions_count = 0;
    VkResult result = VK_SUCCESS;
    
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
    if ((result != VK_SUCCESS) ||
        (extensions_count == 0)) {
      ErrorCheck::setError((char*)"Could not get the number of instance extensions.");
      return false;
    }
    
    available_extensions.resize(extensions_count);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, available_extensions.data());
    if ((result != VK_SUCCESS) ||
        (extensions_count == 0)) {
      ErrorCheck::setError((char *)"Could not enumerate instance extensions.");
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
      if (!LavaCake::Core::IsExtensionSupported(available_extensions, extension)) {
        std::string err = "Extension named '" + std::string(extension) + "' is not supported by an Instance object.";
        ErrorCheck::setError(err.data());
        return false;
      }
    }
    
    /*uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const VkLayerProperties& layerProperties : availableLayers)
    {
      std::cout << layerProperties.layerName << std::endl;
    }*/

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
      ErrorCheck::setError((char *)"Could not create Vulkan instance.");
      return false;
    }
    
    return true;
  }
  
  enum class DeviceValidity {DEVICE_INVALID, DEVICE_EXTENSION_MISSING, DEVICE_VALID};

  DeviceValidity rateLogicalDevice(
                           VkPhysicalDevice                   physical_device,
                           std::vector<char const *> const&   desired_extensions,
                           std::vector<char const*> const&    optional_extensions,
                           std::vector<bool>&                 missing_extension,
                           uint32_t&                          nb_missing_extension) {
    std::vector<VkExtensionProperties> available_extensions;
    
    VkPhysicalDeviceProperties deviceProperties{};
    LavaCake::vkGetPhysicalDeviceProperties(physical_device, &deviceProperties);
    std::string deviceName = std::string(deviceProperties.deviceName);

    uint32_t extensions_count = 0;
    VkResult result = VK_SUCCESS;
    
    result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, nullptr);
    if ((result != VK_SUCCESS) ||
        (extensions_count == 0)) {
      std::string err = "Could not get the number of device extensions for device " + deviceName;
      ErrorCheck::setError(err.data(),5);
      return DeviceValidity::DEVICE_INVALID;
    }
    
    available_extensions.resize(extensions_count);
    result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, available_extensions.data());
    if ((result != VK_SUCCESS) ||
        (extensions_count == 0)) {
      std::string err = "Could not enumerate device extensions for device" + deviceName;
      ErrorCheck::setError(err.data(),5);
      return DeviceValidity::DEVICE_INVALID;
    }
    
    for (auto & extension : desired_extensions) {
      if (!LavaCake::Core::IsExtensionSupported(available_extensions, extension)) {
        std::string err = "Extension named '" + std::string(extension) + "' is not supported by the device " + deviceName;
        ErrorCheck::setError((char*)err.data(),5);
        return DeviceValidity::DEVICE_INVALID;
      }
    }
    
    for (auto& extension : optional_extensions) {
      if (!LavaCake::Core::IsExtensionSupported(available_extensions, extension)) {
        std::string err = "Extension named '" + std::string(extension) + "' is not supported by the device " + deviceName;
        ErrorCheck::setError((char*)err.data(),5);
        missing_extension.push_back(true);
      }
      else {
        missing_extension.push_back(false);
      }
    }

    if (nb_missing_extension != 0) {
      return DeviceValidity::DEVICE_EXTENSION_MISSING;
    }
    
    return DeviceValidity::DEVICE_VALID;
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

    void Device::enableRaytracing(bool optional) {
      m_raytracingEnabled = true;
      m_raytracingOptional = optional;
    }

    void Device::enableMeshShader(bool optional) {
      m_meshShaderEnabled = true;
      m_meshShaderOptional = optional;
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

      if (!WindowParams.headless) {
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
      }
			if (!CreateVulkanInstance(instance_extensions, "LavaCake", m_instance)) {
				ErrorCheck::setError((char*)"Could not create the vulkan instance");
			}

			if (!LavaCake::Core::LoadInstanceLevelFunctions(m_instance, instance_extensions)) {
				ErrorCheck::setError((char*)"Could not load instance level Vulkan functions while initializing the device");
			}

      VkResult result;
      if (!WindowParams.headless) {
        result = glfwCreateWindowSurface(m_instance, WindowParams.Window, nullptr, &m_presentationSurface);
        if (result != VK_SUCCESS || m_presentationSurface == VK_NULL_HANDLE) {
          ErrorCheck::setError((char*)"Failed to create presentation surface");
        }
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
      

      struct tmp_physicalDevice {
        VkPhysicalDevice device;
        uint32_t rating;
        std::vector<bool> missing;
      };

      
      std::vector<tmp_physicalDevice> tmp_physicaldevices;

      for (auto& physical_device : physical_devices) {
        tmp_physicaldevices.push_back({
          physical_device,
          0,
          {}
          });
      }
      std::string err = std::to_string(physical_devices.size()) + " physical device(s) found on this computer.";
      ErrorCheck::setError(err.data(),5);

      std::vector<char const*> device_extensions;

      if (!WindowParams.headless) {
        device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      }
      std::vector<char const*> device_extensions_optional;
      

      if (m_raytracingEnabled) {
        std::vector<char const*>& extension = m_raytracingOptional ? device_extensions_optional : device_extensions;

        extension.insert(extension.end(), raytracingExtension.begin(), raytracingExtension.end());

      }

      if (0) { // ray query

        //device_extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
      }

      if (m_meshShaderEnabled) {
        std::vector<char const*>& extension = m_meshShaderOptional ? device_extensions_optional : device_extensions;

        extension.insert(extension.end(), meshShaderExtension.begin(), meshShaderExtension.end());
        

      }


			for (int i = 0; i < nbComputeQueue; i++) {
				m_computeQueues.push_back(ComputeQueue());
			}

			for (int i = 0; i < nbGraphicQueue; i++) {
				m_graphicQueues.push_back(GraphicQueue());
			}


      //rate devices
      std::vector<uint32_t> deviceRating;
      std::vector<std::vector<bool>> missingExtensionPerDevices;
			for (auto& physicalDevice : tmp_physicaldevices) {
				
        std::vector<bool> missingExtension;
        uint32_t nbMissingExtension;
        DeviceValidity dv = rateLogicalDevice(
          physicalDevice.device,
          device_extensions,
          device_extensions_optional,
          missingExtension,
          nbMissingExtension);
        
        if (dv != DeviceValidity::DEVICE_INVALID) {
          physicalDevice.missing = missingExtension;
          physicalDevice.rating = nbMissingExtension;
        }
        else {
          physicalDevice.rating  = UINT32_MAX;
        }

			}

      //sort devices by theire rate :
      std::sort(tmp_physicaldevices.begin(), tmp_physicaldevices.end(), 
        [](const tmp_physicalDevice& a, const tmp_physicalDevice& b) {
          return a.rating < b.rating;
        });


      //brows all compatible device

      struct QueueInfo {
        uint32_t           FamilyIndex;
        std::vector<float> Priorities;
      };

      for (auto& device : tmp_physicaldevices) {
        std::vector<const char*> extensionToLoad(device_extensions);
        // create 
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        std::vector<QueueInfo> requested_queues;
        for (int i = 0; i < nbGraphicQueue; i++) {
          if (!m_graphicQueues[i].initIndex(device.device)) {
            goto endloop;
          }
        }

        for (int i = 0; i < nbComputeQueue; i++) {
          if (!m_computeQueues[i].initIndex(device.device)) {
            goto endloop;
          }
        }

        if (!WindowParams.headless) {
          if (!m_presentQueue->initIndex(device.device, &m_presentationSurface)) {
            continue;
          }
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

        if (!WindowParams.headless) {
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
        }
      endConcPresent:;


        if (desired_device_features == nullptr) {
          desired_device_features = new VkPhysicalDeviceFeatures();
        }

        for (auto& info : requested_queues) {
          queue_create_infos.push_back({
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,       // VkStructureType                  sType
            nullptr,                                          // const void                     * pNext
            0,                                                // VkDeviceQueueCreateFlags         flags
            info.FamilyIndex,                                 // uint32_t                         queueFamilyIndex
            static_cast<uint32_t>(info.Priorities.size()),    // uint32_t                         queueCount
            info.Priorities.data()                            // const float                    * pQueuePriorities
            });
        };

        for (size_t s = 0; s < device_extensions_optional.size(); s++) {
          if (!device.missing[s]) {
            extensionToLoad.push_back(device_extensions_optional[s]);
          }
        }
        {
          VkDeviceCreateInfo device_create_info = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,               // VkStructureType                  sType
            nullptr,                                            // const void                     * pNext
            0,                                                  // VkDeviceCreateFlags              flags
            static_cast<uint32_t>(queue_create_infos.size()),   // uint32_t                         queueCreateInfoCount
            queue_create_infos.data(),                          // const VkDeviceQueueCreateInfo  * pQueueCreateInfos
            0,                                                  // uint32_t                         enabledLayerCount
            nullptr,                                            // const char * const             * ppEnabledLayerNames
            static_cast<uint32_t>(extensionToLoad.size()),      // uint32_t                         enabledExtensionCount
            extensionToLoad.data(),                             // const char * const             * ppEnabledExtensionNames
            desired_device_features                             // const VkPhysicalDeviceFeatures * pEnabledFeatures
          };

          


          std::vector<const char*> missingOptionalExtension;
          for (size_t s = 0; s < device_extensions_optional.size(); s++) {
            if (device.missing[s]) {
              missingOptionalExtension.push_back(device_extensions_optional[s]);
            }
          }

          bool raytracingAvailable = m_raytracingEnabled;
          bool meshShaderAvailable = m_meshShaderEnabled;

          for (auto e : missingOptionalExtension) {
            if (m_raytracingEnabled && m_raytracingOptional && raytracingAvailable) {
              for (auto rte : raytracingExtension) {
                if (e == rte) {
                  raytracingAvailable = false;
                  break;
                }
              }
            }
            if (m_meshShaderEnabled && m_meshShaderOptional && meshShaderAvailable) {
              for (auto rte : meshShaderExtension) {
                if (e == rte) {
                  meshShaderAvailable = false;
                  //ErrorCheck::setError((char*)"Raytracing extensions not found on this device", 1);
                  break;
                }
              }
            }
          }

          VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
          VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
          VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
          VkPhysicalDeviceMeshShaderFeaturesNV enabledMeshShaderFeatures{};

          void* pNextChain = nullptr;

          if (raytracingAvailable) {

            enabledBufferDeviceAddresFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
            enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;

            enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
            enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
            enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddresFeatures;

            enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
            enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;

            pNextChain = &enabledAccelerationStructureFeatures;
            
          }

          if (meshShaderAvailable) {
            enabledMeshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
            enabledMeshShaderFeatures.pNext = pNextChain;
            enabledMeshShaderFeatures.taskShader = VK_TRUE;
            enabledMeshShaderFeatures.meshShader = VK_TRUE;

            pNextChain = &enabledMeshShaderFeatures;
          }

          VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
          if (pNextChain) {
            physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            physicalDeviceFeatures2.features = *desired_device_features;
            physicalDeviceFeatures2.pNext = pNextChain;
            device_create_info.pEnabledFeatures = nullptr;
            device_create_info.pNext = &physicalDeviceFeatures2;
          }


          result = vkCreateDevice(device.device, &device_create_info, nullptr, &m_logical);
          if ((result != VK_SUCCESS) ||
            (m_logical == VK_NULL_HANDLE)) {
            continue;
          }
          else {
            m_physical = device.device;

            m_raytracingAvailable = raytracingAvailable;
            m_meshShaderAvailable = meshShaderAvailable;
            

            

            VkPhysicalDeviceProperties deviceProperties{};
            LavaCake::vkGetPhysicalDeviceProperties(m_physical, &deviceProperties);
            err = "Chosen device is " + std::string(deviceProperties.deviceName);
            ErrorCheck::setError(err.data(),5);

            if (!m_raytracingAvailable) {
              ErrorCheck::setError((char*)"Raytracing extensions not found on this device", 1);
            }
            if (!m_meshShaderAvailable) {
              ErrorCheck::setError((char*)"Mesh shader extensions not found on this device", 1);
            }

            LavaCake::Core::LoadDeviceLevelFunctions(m_logical, extensionToLoad);


            for (int i = 0; i < nbGraphicQueue; i++) {
              LavaCake::vkGetDeviceQueue(m_logical, m_graphicQueues[i].getIndex(), 0, &m_graphicQueues[i].getHandle());
            }

            for (int i = 0; i < nbComputeQueue; i++) {
              LavaCake::vkGetDeviceQueue(m_logical, m_computeQueues[i].getIndex(), 0, &m_computeQueues[i].getHandle());
            }

            if (!WindowParams.headless) {
              LavaCake::vkGetDeviceQueue(m_logical, m_presentQueue->getIndex(), 0, &m_presentQueue->getHandle());
            }
            //Todo Check if getHandle()  works

            


            break;
          }
        }
        endloop:;
      }
      
			if (!m_logical) {
				ErrorCheck::setError((char*)"The logical device could not be created");
        return;
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

      if (VK_NULL_HANDLE != m_commandPool) {
        vkDestroyCommandPool(m_logical, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
      }

      //release logical device
      vkDestroyDevice(m_logical, nullptr);


      //release Vulkan lib
      if (nullptr != m_vulkanLibrary) {
#if defined _WIN32
        FreeLibrary(m_vulkanLibrary);
#elif defined __linux
        dlclose(m_vulkanLibrary);
#elif defined __APPLE__
        dlclose(m_vulkanLibrary);
#endif
        m_vulkanLibrary = nullptr;
      }

      delete m_device;
		}
  }
}
