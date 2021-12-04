#include "Device.h"

namespace LavaCake {
  namespace Framework {
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


		void Device::initDevices(int nbComputeQueue, int nbGraphicQueue, WindowParameters&	WindowParams, VkPhysicalDeviceFeatures* desired_device_features) {



			if (!LavaCake::Core::ConnectWithVulkanLoaderLibrary(m_vulkanLibrary)) {
				ErrorCheck::setError((char*)"Could not connect with Vulkan while initializing the device");
			}

			if (!LavaCake::Core::LoadFunctionExportedFromVulkanLoaderLibrary(m_vulkanLibrary)) {
				ErrorCheck::setError((char*)"Could not load Vulkan library while initializing the device");
			}

			if (!LavaCake::Core::LoadGlobalLevelFunctions()) {
				ErrorCheck::setError((char*)"Could not load global level Vulkan functions while initializing the device");
			}



			

			std::vector<char const*> instance_extensions;
			instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

			
			if (!LavaCake::Core::CreateVulkanInstanceWithWsiExtensionsEnabled(instance_extensions, "LavaCake", m_instance)) {
				ErrorCheck::setError((char*)"Could not load Vulkan while initializing the device");
			}

			if (!LavaCake::Core::LoadInstanceLevelFunctions(m_instance, instance_extensions)) {
				ErrorCheck::setError((char*)"Could not load instance level Vulkan functions while initializing the device");
			}

			if (!LavaCake::Core::CreatePresentationSurface(m_instance, WindowParams, m_presentationSurface)) {
				ErrorCheck::setError((char*)"Failed to create presentation surface");
			}

			std::vector<VkPhysicalDevice> physical_devices;
			LavaCake::Core::EnumerateAvailablePhysicalDevices(m_instance, physical_devices);

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
				std::vector <LavaCake::Core::QueueInfo > requested_queues;
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


#ifdef RAYTRACING
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
#endif // USE_NV_RAYTRACING

#ifdef RAYQUERY
				//device_extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
#endif // RAYQUERY

				device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

				if (desired_device_features == nullptr) {
					desired_device_features = new VkPhysicalDeviceFeatures();
				}
				if (!LavaCake::Core::CreateLogicalDevice(physical_device, requested_queues, device_extensions, desired_device_features, (void*)(&enabledAccelerationStructureFeatures), m_logical)) {
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

			if (!LavaCake::Core::CreateCommandPool(m_logical, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_graphicQueues[0].getIndex(), m_commandPool)) {
				ErrorCheck::setError((char*)"The command pool could not be created");
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
		}


		

  }
}
