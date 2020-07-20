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

			static Device* getDevice() {
				if (!m_device) {
					m_device = new Device();
				}
				return m_device;
			}

			VkPhysicalDevice& getPhysicalDevice();

			VkDevice& getLogicalDevice();

			SwapChain& getSwapChain();

			std::vector<Buffer::FrameResources>* getFrameRessources();

			PresentationQueue* getPresentQueue();

			GraphicQueue* getGraphicQueue(int i);

			void initDevices(VkPhysicalDeviceFeatures * desired_device_features, int nbComputeQueue, int nbGraphicQueue, WindowParameters	WindowParams);

			void prepareFrames(uint32_t									 framesCount,
				VkImageUsageFlags          swapchain_image_usage,
				bool                       use_depth,
				VkImageUsageFlags          depth_attachment_usage);


			//Make sure every command send to the device are finished
			void end();


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