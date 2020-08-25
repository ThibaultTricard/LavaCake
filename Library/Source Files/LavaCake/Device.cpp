#include "Device.h"

namespace LavaCake {
  namespace Framework {
		Device* Device::m_device;

		VkPhysicalDevice& Device::getPhysicalDevice() {
			return m_physical;
		}

		VkDevice& Device::getLogicalDevice() {
			return *m_logical;
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
			return *m_commandPool;
		};

		VkSurfaceKHR  Device::getSurface() {
			return *m_presentationSurface;
		};


		void Device::initDevices(int nbComputeQueue, int nbGraphicQueue, WindowParameters	WindowParams, VkPhysicalDeviceFeatures* desired_device_features) {
			if (!Loader::ConnectWithVulkanLoaderLibrary(m_vulkanLibrary)) {
				ErrorCheck::setError("Could not connect with Vulkan while initializing the device");
			}

			if (!Loader::LoadFunctionExportedFromVulkanLoaderLibrary(m_vulkanLibrary)) {
				ErrorCheck::setError("Could not load Vulkan library while initializing the device");
			}

			if (!Loader::LoadGlobalLevelFunctions()) {
				ErrorCheck::setError("Could not load global level Vulkan functions while initializing the device");
			}



			std::vector<char const*> instance_extensions;
			InitVkDestroyer(m_instance);
			if (!Instance::CreateVulkanInstanceWithWsiExtensionsEnabled(instance_extensions, "LavaCake", *m_instance)) {
				ErrorCheck::setError("Could not load Vulkan while initializing the device");
			}

			if (!Loader::LoadInstanceLevelFunctions(*m_instance, instance_extensions)) {
				ErrorCheck::setError("Could not load instance level Vulkan functions while initializing the device");
			}

			InitVkDestroyer(m_instance, m_presentationSurface);
			if (!Presentation::CreatePresentationSurface(*m_instance, WindowParams, *m_presentationSurface)) {
				ErrorCheck::setError("Failed to create presentation surface");
			}

			std::vector<VkPhysicalDevice> physical_devices;
			LavaCake::Device::Physical::EnumerateAvailablePhysicalDevices(*m_instance, physical_devices);

			for (int i = 0; i < nbComputeQueue; i++) {
				m_computeQueues.push_back(ComputeQueue());
			}

			for (int i = 0; i < nbGraphicQueue; i++) {
				m_graphicQueues.push_back(GraphicQueue());
			}

			for (auto& physical_device : physical_devices) {
				std::vector<char const*> device_extensions;
				std::vector <LavaCake::Queue::QueueInfo > requested_queues;
				for (int i = 0; i < nbGraphicQueue; i++) {
					if (!m_graphicQueues[0].initIndex(&physical_device)) {
						goto endloop;
					}
				}

				for (int i = 0; i < nbComputeQueue; i++) {
					if (!m_computeQueues[i].initIndex(&physical_device)) {
						goto endloop;
					}
				}

				if (!m_presentQueue->initIndex(&physical_device, &(*m_presentationSurface))) {
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


				InitVkDestroyer(m_logical);
				if (!Instance::CreateLogicalDeviceWithWsiExtensionsEnabled(physical_device, requested_queues, device_extensions, desired_device_features, *m_logical)) {
					continue;
				}
				else {
					m_physical = physical_device;
					Loader::LoadDeviceLevelFunctions(*m_logical, device_extensions);
					//Todo Check if getHandle()  works
					for (int i = 0; i < nbGraphicQueue; i++) {
						LavaCake::vkGetDeviceQueue(*m_logical, m_graphicQueues[i].getIndex(), 0, &m_graphicQueues[i].getHandle());
					}

					for (int i = 0; i < nbComputeQueue; i++) {
						LavaCake::vkGetDeviceQueue(*m_logical, m_computeQueues[i].getIndex(), 0, &m_computeQueues[i].getHandle());
					}

					LavaCake::vkGetDeviceQueue(*m_logical, m_presentQueue->getIndex(), 0, &m_presentQueue->getHandle());
					break;
				}

			endloop:;
			}

			if (!m_logical) {
				ErrorCheck::setError("The logical device could not be created");
			}


			InitVkDestroyer(m_logical, m_commandPool);
			if (!Command::CreateCommandPool(*m_logical, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_graphicQueues[0].getIndex(), *m_commandPool)) {
				ErrorCheck::setError("The command pool could not be created");
			}
		}

		

		void Device::end() {
			if (m_logical) {
				Command::WaitForAllSubmittedCommandsToBeFinished(*m_logical);
			}
		}


		

  }
}