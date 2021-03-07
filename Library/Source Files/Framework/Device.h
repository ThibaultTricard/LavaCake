#pragma once
#include "AllHeaders.h"
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

			VkInstance getInstance() {
				return *m_instance;
			};

			VkPhysicalDevice& getPhysicalDevice();

			VkDevice& getLogicalDevice();

			VkCommandPool getCommandPool();

			VkSurfaceKHR getSurface();

			PresentationQueue* getPresentQueue();

			GraphicQueue* getGraphicQueue(int i);

			ComputeQueue* getComputeQueue(int i);

			void initDevices( int nbComputeQueue, int nbGraphicQueue, WindowParameters&	WindowParams, VkPhysicalDeviceFeatures * desired_device_features = nullptr);
 
			//Make sure every command send to the device are finished
			void end();

			

			private :
				VkPhysicalDevice													m_physical = VK_NULL_HANDLE;
				VkDestroyer(VkDevice)											m_logical;
				LIBRARY_TYPE															m_vulkanLibrary;
				VkDestroyer(VkInstance)										m_instance;
				VkDestroyer(VkSurfaceKHR)                 m_presentationSurface;
				VkDestroyer(VkCommandPool)								m_commandPool;
				std::vector<GraphicQueue>									m_graphicQueues;
				std::vector<ComputeQueue>									m_computeQueues;
				PresentationQueue*												m_presentQueue = new PresentationQueue();
		};
	}
}