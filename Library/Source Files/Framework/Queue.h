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

      
			Queue() {
				m_handle = new VkQueue();
			}

			virtual bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr) { return false; };

      /**
       \brief Get the handle of the Queue
       */
			VkQueue& getHandle() {
				return *m_handle;
			}

			uint32_t getIndex() {
				return m_familyIndex;
			}

			void setHandle(VkQueue*	handle) {
				m_handle = handle;
			}

			~Queue() {

			}

		protected:
			VkQueue*		m_handle ;
			uint32_t		m_familyIndex =0;
		};


  /**
   Class ComputeQueue :
   \brief A class to help manage compute VkQueue
   */
		class ComputeQueue : public Queue {
		public:
			virtual bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr) override {
				return LavaCake::Core::SelectIndexOfQueueFamilyWithDesiredCapabilities(*physicalDevice, VK_QUEUE_GRAPHICS_BIT, m_familyIndex);
			}
		};

  /**
   Class GraphicQueue :
   \brief A class to help manage graphique VkQueue
   */
		class GraphicQueue : public Queue {
		public:
			virtual bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr) override {
				return LavaCake::Core::SelectIndexOfQueueFamilyWithDesiredCapabilities(*physicalDevice, VK_QUEUE_COMPUTE_BIT, m_familyIndex);
			}
		};
  
  /**
   Class PresentationQueue :
   \brief A class to help manage present VkQueue
   */
		class PresentationQueue : public Queue {
		public:
			virtual bool initIndex(VkPhysicalDevice* physicalDevice, VkSurfaceKHR* surface = nullptr) override {
        std::vector<VkQueueFamilyProperties> queue_families;
        uint32_t queue_families_count = 0;
          
        vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &queue_families_count, nullptr);
        if (queue_families_count == 0) {
          ErrorCheck::setError((char*)"Could not get the number of queue families.");
          return false;
        }
        
        queue_families.resize(queue_families_count);
        vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &queue_families_count, queue_families.data());
        if (queue_families_count == 0) {
          ErrorCheck::setError((char*) "Could not acquire properties of queue families.");
          return false;
        }

        for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index) {
          VkBool32 presentation_supported = VK_FALSE;
          VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(*physicalDevice, index, *surface, &presentation_supported);
          if ((VK_SUCCESS == result) &&
              (VK_TRUE == presentation_supported)) {
            m_familyIndex = index;
            return true;
          }
        }
        return false;
			}
		};
	}
}
