#pragma once

#include "AllHeaders.h"
#include "VulkanDestroyer.h"
#include "SwapChain.h"

#include "Tools.h"

namespace LavaCake {
	namespace Framework {
		class Queue {
		public :

			Queue() {
				m_handle = new VkQueue();
			}

			bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr);

			VkQueue& getHandle() {
				return *m_handle;
			}

			uint32_t getIndex() {
				return m_familyIndex;
			}

			void setHandle(VkQueue*		handle) {
				m_handle = handle;
			}

		protected:
			VkQueue*		m_handle ;
			uint32_t		m_familyIndex;
		};


		class ComputeQueue : public Queue {
		public:
			bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr) {
				return LavaCake::Queue::SelectIndexOfQueueFamilyWithDesiredCapabilities(*physicalDevice, VK_QUEUE_GRAPHICS_BIT, m_familyIndex);
			}
		};

		class GraphicQueue : public Queue {
		public:
			bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr) {
				return LavaCake::Queue::SelectIndexOfQueueFamilyWithDesiredCapabilities(*physicalDevice, VK_QUEUE_COMPUTE_BIT, m_familyIndex);
			}
		};

		class PresentationQueue : public Queue {
		public:
			virtual bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr) {
				return Presentation::SelectQueueFamilyThatSupportsPresentationToGivenSurface(*physicalDevice, *surface, m_familyIndex);
			}
		};
	}
}