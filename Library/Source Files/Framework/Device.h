#pragma once
#include "AllHeaders.h"
#include "Queue.h"
#include "ErrorCheck.h"


namespace LavaCake {
  namespace Framework {

  /**
   Class Device :
   \brief helps manage Vulkan device related task
   This class is a singleton
   */
		class Device {
			static Device* m_device;
			Device() {};
      ~Device() {
        /*vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
        vkDestroyDevice(m_logical, nullptr);
        m_logical = VK_NULL_HANDLE;*/
      }
		public:
      /**
       \brief Retourn the  device
       \return a static reference to the devince
       */
			static Device* getDevice() {
				if (!m_device) {
					m_device = new Device();
				}
				return m_device;
			}

      /**
       \brief Retourn the Vulkan instance
       \return the VkInstance used by the application
       */
			VkInstance getInstance() {
				return m_instance;
			};

      /**
       \brief Retourn the Physical Device
       \return the VkPhysicalDevice used by the application
       */
			VkPhysicalDevice& getPhysicalDevice();

      /**
       \brief Retourn the Logical Device
       \return the VkDevice used by the application
       */
			VkDevice& getLogicalDevice();

      /**
       \brief Retourn the Command pool
       \return the VkCommandPool used by the application
       */
			VkCommandPool getCommandPool();

      /**
       \brief Retourn the Vulkan surface
       \return the VkSurfaceKHR used by the application
       */
			VkSurfaceKHR getSurface();

      /**
       \brief Retourn the Presentation Queue used to draw on the screen
       \return a reference to PresentationQueue used by the application
       */
			PresentationQueue* getPresentQueue();

      /**
       \brief Retourn a specific Graphic Queue
       \param i the index of the wanted queue
       \return a reference to a GraphicQueue
       */
			GraphicQueue* getGraphicQueue(int i);

      /**
       \brief Retourn a specific Compute Queue
       \param i the index of the wanted queue
       \return a reference to a ComputeQueue
       */
			ComputeQueue* getComputeQueue(int i);

      /**
       \brief Initialise the device
       \param nbComputeQueue the number of compute queue requiered by the application
       \param nbGraphicQueue the number of graphic queue requiered by the application
       \param windowParams the windows parameter of the application
       \param desiredDeviceFeatures the device feature requiered by the application
       \return a reference to a ComputeQueue
       */
			void initDevices( int nbComputeQueue, int nbGraphicQueue, WindowParameters&	windowParams, VkPhysicalDeviceFeatures * desiredDeviceFeatures = nullptr);
      
      /**
       \brief free all the vulkan handle hold by the device and destroy the singleton 
      */
			void end();
      
      /**
       \brief Make sure every command send to the device are finished
       */
			void waitForAllCommands();
			

      /**
       \brief Ask the device to enable all feature related to raytracing
       \param optional: a boolean indicating if the device creation can success even if raytracing feature are not found
       */
      void enableRaytracing(bool optional = false);

      void enableMeshShader(bool optional = false);

      /*
        \brief check if raytracing features have been successfuly loaded
        \return true if raytracing is available
      */
      bool raytracingAvailable() {
        return m_raytracingAvailable;
      }

      /*
        \brief check if mesh shader features have been successfuly loaded
        \return true if mesh shader is available
      */
      bool meshShaderAvailable() {
        return m_meshShaderAvailable;
      }

			private :

        
				VkPhysicalDevice													m_physical = VK_NULL_HANDLE;
				VkDevice            											m_logical = VK_NULL_HANDLE;
				LIBRARY_TYPE															m_vulkanLibrary;
				VkInstance            										m_instance = VK_NULL_HANDLE;
				VkSurfaceKHR                              m_presentationSurface = VK_NULL_HANDLE;
				VkCommandPool					              			m_commandPool = VK_NULL_HANDLE;
				std::vector<GraphicQueue>									m_graphicQueues;
				std::vector<ComputeQueue>									m_computeQueues;
				PresentationQueue*												m_presentQueue = new PresentationQueue();

        
        bool                                      m_raytracingEnabled = false;
        bool                                      m_raytracingOptional = true;
        bool                                      m_meshShaderEnabled = false;
        bool                                      m_meshShaderOptional = true;

        std::vector<const char*>                  m_missingOptionalExtension;

        bool                                      m_raytracingAvailable = false;
        bool                                      m_meshShaderAvailable = false;
		};
	}
}
