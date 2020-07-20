#pragma once
#include "AllHeaders.h"
#include "SwapChain.h"
#include "Queue.h"
#include "ErrorCheck.h"


namespace LavaCake {
	namespace Framework {

		class Device {
			static Device* m_device;
			Device() {};

		public:

			static Device* getDevice(){
				if (!m_device) {
					m_device = new Device();
				}
				return m_device;
			}

			VkPhysicalDevice& getPhysicalDevice() {
				return m_physical;
			}

			VkDevice& getLogicalDevice() {
				return *m_logical;
			}

			SwapChain& getSwapChain() {
				return *m_swapChain;
			}

			std::vector<Buffer::FrameResources>* getFrameRessources() {
				return &framesResources;
			}
			PresentationQueue& getPresentQueue() {  
				return *m_presentQueue;
			};

			GraphicQueue* getGraphicQueue(int i) {
				return &m_graphicQueues[i];
			};

			void initDevices(VkPhysicalDeviceFeatures * desired_device_features, int nbComputeQueue, int nbGraphicQueue, WindowParameters	WindowParams) {
				if (!Loader::ConnectWithVulkanLoaderLibrary(m_vulkanLibrary)) {
          ErrorCheck::setError(2, "cannot connect with Vulkan");
				}

				if (!Loader::LoadFunctionExportedFromVulkanLoaderLibrary(m_vulkanLibrary)) {
          ErrorCheck::setError(2, "cannot load Vulkan");
				}

				if (!Loader::LoadGlobalLevelFunctions()) {
          ErrorCheck::setError(2, "cannot load global level Vulkan functions");
				}



				std::vector<char const *> instance_extensions;
				InitVkDestroyer(m_instance);
				if (!Instance::CreateVulkanInstanceWithWsiExtensionsEnabled(instance_extensions, "LavaCake", *m_instance)) {
          ErrorCheck::setError(2, "cannot load Vulkan instance");
				}

				if (!Loader::LoadInstanceLevelFunctions(*m_instance, instance_extensions)) {
          ErrorCheck::setError(2, "cannot load instance level Vulkan functions");
				}

				InitVkDestroyer(m_instance, m_presentationSurface);
				if (!Presentation::CreatePresentationSurface(*m_instance, WindowParams, *m_presentationSurface)) {
          ErrorCheck::setError(2, "cannot create presentation surface");
				}

				std::vector<VkPhysicalDevice> physical_devices;
				LavaCake::Device::Physical::EnumerateAvailablePhysicalDevices(*m_instance, physical_devices);

				for (int i = 0; i < nbComputeQueue; i++) {
					m_computeQueues.push_back(ComputeQueue());
				}

				for (int i = 0; i < nbGraphicQueue; i++) {
					m_graphicQueues.push_back(GraphicQueue());
				}

				for (auto & physical_device : physical_devices) {
					std::vector<char const *> device_extensions;
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
          ErrorCheck::setError(2, "logical device not created");
				}


				InitVkDestroyer(m_logical, m_commandPool);
				if (!Command::CreateCommandPool(*m_logical, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_graphicQueues[0].getIndex(), *m_commandPool)) {
          ErrorCheck::setError(2, "cannot create commade pool");
				}
			}

			void prepareFrames(uint32_t									 framesCount,
				VkImageUsageFlags          swapchain_image_usage,
				bool                       use_depth,
				VkImageUsageFlags          depth_attachment_usage) {
				for (uint32_t i = 0; i < framesCount; ++i) {
					std::vector<VkCommandBuffer> command_buffer;
					VkDestroyer(VkSemaphore) image_acquired_semaphore;
					InitVkDestroyer(m_logical, image_acquired_semaphore);
					VkDestroyer(VkSemaphore) ready_to_present_semaphore;
					InitVkDestroyer(m_logical, ready_to_present_semaphore);
					VkDestroyer(VkFence) drawing_finished_fence;
					InitVkDestroyer(m_logical, drawing_finished_fence);
					VkDestroyer(VkImageView) depth_attachment;
					InitVkDestroyer(m_logical, depth_attachment);

					if (!Command::AllocateCommandBuffers(*m_logical, *m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, command_buffer)) {
            ErrorCheck::setError(2, "cannot allocate commande buffer");
					}
					if (!Semaphore::CreateSemaphore(*m_logical, *image_acquired_semaphore)) {
            ErrorCheck::setError(2, "cannot create commande semaphore");
					}
					if (!Semaphore::CreateSemaphore(*m_logical, *ready_to_present_semaphore)) {
            ErrorCheck::setError(2, "cannot create commande semaphore");
					}
					if (!Fence::CreateFence(*m_logical, true, *drawing_finished_fence)) {
            ErrorCheck::setError(2, "cannot create commande dence");
					}

					framesResources.emplace_back(
						command_buffer[0],
						std::move(image_acquired_semaphore),
						std::move(ready_to_present_semaphore),
						std::move(drawing_finished_fence),
						std::move(depth_attachment),
						VkDestroyer(VkFramebuffer)()
					);
				}

				m_swapChain->init(framesCount, *m_logical, m_physical, *m_presentationSurface, framesResources, swapchain_image_usage, use_depth, depth_attachment_usage);
			}

			void end() {
				if (m_logical) {
					Command::WaitForAllSubmittedCommandsToBeFinished(*m_logical);
				}
			}


			private :
				VkPhysicalDevice													m_physical;
				VkDestroyer(VkDevice)											m_logical;
				LIBRARY_TYPE															m_vulkanLibrary;
				VkDestroyer(VkInstance)										m_instance;
				VkDestroyer(VkSurfaceKHR)                 m_presentationSurface;
				VkDestroyer(VkCommandPool)                m_commandPool;
				PresentationQueue*												m_presentQueue = new PresentationQueue();
				std::vector<GraphicQueue>									m_graphicQueues;
				std::vector<ComputeQueue>									m_computeQueues;
				SwapChain*																m_swapChain = new SwapChain();
				std::vector<Buffer::FrameResources>				framesResources;
			
		};
	}
}