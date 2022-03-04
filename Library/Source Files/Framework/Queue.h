#pragma once

#include "AllHeaders.h"
#include "ErrorCheck.h"
#include "Tools.h"

namespace LavaCake {
	namespace Framework {
  /**
   Class Queue :
   \brief A gerneric class to help manage VkQueue inherited by the classes ComputeQueue, GraphicQueue, and PresentationQueue
   */
		class Queue {
		public:

      
      Queue() : m_handle() {
			}

      virtual bool initIndex(VkPhysicalDevice& /*physicalDevice*/, VkSurfaceKHR* surface = nullptr) { return false; };

      /**
       \brief Get the handle of the Queue
       */
			const VkQueue& getHandle() const{
        return m_handle;
			}

			void setHandle(VkQueue queue) {
				m_handle = queue;
			}

			uint32_t getIndex() const{
				return m_familyIndex;
			}


      virtual ~Queue() = default;

		protected:

      bool setIndex(VkPhysicalDevice& physicalDevice, VkQueueFlags  queueFlags, VkSurfaceKHR* surface = nullptr) {
				std::vector<VkQueueFamilyProperties> queue_families;

				uint32_t queueFamiliesCount = 0;

				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
				if (queueFamiliesCount == 0) {
					ErrorCheck::setError("Could not get the number of queue families.");
					return false;
				}

				queue_families.resize(queueFamiliesCount);
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queue_families.data());
				if (queueFamiliesCount == 0) {
					ErrorCheck::setError("Could not acquire properties of queue families.");
					return false;
				}

				for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index) {
					if (queueFlags == 0) { // Present Queue
						VkBool32 presentation_supported = VK_FALSE;
						if (surface == nullptr) {
							ErrorCheck::setError("Invalid surface pointer");
							return false;
						}
						VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, *surface, &presentation_supported);
						if ((VK_SUCCESS == result) &&
							(VK_TRUE == presentation_supported)) {
							m_familyIndex = index;
							return true;
						}
					}
					else { // Compute and Graphic Queue
						if ((queue_families[index].queueCount > 0) &&
							((queue_families[index].queueFlags & queueFlags) == queueFlags)) {
							m_familyIndex = index;
							return true;
						}
					}
				}
				return false;
      }

      VkQueue  		m_handle ;
			uint32_t		m_familyIndex =0;
		};


  /**
   Class ComputeQueue :
   \brief A class to help manage compute VkQueue
   */
		class ComputeQueue : public Queue {
		public:
			virtual bool initIndex(VkPhysicalDevice& physicalDevice, VkSurfaceKHR* surface = nullptr) override {
				return setIndex(physicalDevice, VK_QUEUE_COMPUTE_BIT);
			}
		};

  /**
   Class GraphicQueue :
   \brief A class to help manage graphique VkQueue
   */
		class GraphicQueue : public Queue {
		public:
			virtual bool initIndex(VkPhysicalDevice& physicalDevice, VkSurfaceKHR* surface = nullptr) override {
				return setIndex(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
			}
		};
  
  /**
   Class PresentationQueue :
   \brief A class to help manage present VkQueue
   */
		class PresentationQueue : public Queue {
		public:
			virtual bool initIndex(VkPhysicalDevice& physicalDevice, VkSurfaceKHR* surface = nullptr) override {
        return setIndex(physicalDevice, 0, surface);
			}
		};
	}
}
