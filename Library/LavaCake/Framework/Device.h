#pragma once
#include "AllHeaders.h"
#include "Queue.h"
#include "ErrorCheck.h"


#if defined(LAVACAKE_WINDOW_MANAGER_GLFW)
#define GLFW_INCLUDE_NONE
#ifdef _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32 true
#elif __APPLE__
#define GLFW_INCLUDE_VULKAN
#endif

#include "glfw3.h"
#include "glfw3native.h"

#undef interface

#elif defined(LAVACAKE_WINDOW_MANAGER_CUSTOM)

#endif



namespace LavaCake {
  namespace Framework {
    /**
     \brief Helps the Device class initialize the vulkan surface
     */
    class SurfaceInitialisator {
    public:
      /**
       \brief initialize the vulkan surface
       \param instance: the Vulkan instance used to initialize the surface
       \return a VkSurface
       */
      virtual VkSurfaceKHR init(const VkInstance& instance) = 0;
    };
    
  
  class HeadlessInitialisator : public SurfaceInitialisator {
    public:
    
    HeadlessInitialisator(){}
    /**
     \brief initialize the vulkan surface
     \param instance: the Vulkan instance used to initialize the surface
     \return a VkSurface
     */
    VkSurfaceKHR init(const VkInstance& instance) override{
      return {};
    };
  };

#ifdef LAVACAKE_WINDOW_MANAGER_GLFW
    /**
     \brief Helps the Device class initialize the vulkan surface with GLFW.
     */
    class GLFWSurfaceInitialisator : public SurfaceInitialisator {
    public:

      /**
       \brief constructor for GLFWSurfaceInitialisator
       \param window a poiter to the GLFWwindow
       */
      GLFWSurfaceInitialisator(GLFWwindow* window) {
        m_window = window;
      }

      /**
       \brief initialize the vulkan surface for GLFW
       \param instance: the Vulkan instance used to initialize the surface
       \return a VkSurface
       */
      VkSurfaceKHR init(const VkInstance& instance) override {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult result = glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
        if (result != VK_SUCCESS || surface == VK_NULL_HANDLE) {
          ErrorCheck::setError("Failed to create presentation surface");
        }
        return surface;
      }

    private:
      GLFWwindow* m_window = nullptr;
    };
#endif // LAVACAKE_WINDOW_MANAGER_GLFW

  }
}

namespace LavaCake {
  namespace Framework {

    /**
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
      const VkInstance getInstance() const {
        return m_instance;
      };

      /**
      \brief Retourn the Physical Device
      \return the VkPhysicalDevice used by the application
      */
      const VkPhysicalDevice& getPhysicalDevice() const;

      /**
      \brief Retourn the Logical Device
      \return the VkDevice used by the application
      */
      const VkDevice& getLogicalDevice() const;

      /**
      \brief Retourn the Command pool
      \return the VkCommandPool used by the application
      */
      const VkCommandPool& getCommandPool() const;

      /**
      \brief Retourn the Vulkan surface
      \return the VkSurfaceKHR used by the application
      */
      const VkSurfaceKHR& getSurface() const;

      /**
      \brief Retourn the Presentation Queue used to draw on the screen
      \return a reference to PresentationQueue used by the application
      */
      const PresentationQueue& getPresentQueue() const;

      /**
      \brief Retourn a specific Graphic Queue
      \param i the index of the wanted queue
      \return a reference to a GraphicQueue
      */
      const GraphicQueue& getGraphicQueue(int i) const;

      /**
      \brief Retourn a specific Compute Queue
      \param i the index of the wanted queue
      \return a reference to a ComputeQueue
      */
      const ComputeQueue& getComputeQueue(int i)const;



      /**
      \brief Initialise the device
      \param nbComputeQueue the number of compute queue requiered by the application
      \param nbGraphicQueue the number of graphic queue requiered by the application
      \param window the helper for surface initialisation
      \param desiredDeviceFeatures the device feature requiered by the application
      \return a reference to a ComputeQueue
      */
      void initDevices(
        int nbComputeQueue,
        int nbGraphicQueue,
        SurfaceInitialisator& window,
        VkPhysicalDeviceFeatures* desiredDeviceFeatures = nullptr){
        initDevices(nbComputeQueue, nbGraphicQueue, window, false, desiredDeviceFeatures);
      }

      /**
      \brief Initialise the device
      \param nbComputeQueue the number of compute queue requiered by the application
      \param nbGraphicQueue the number of graphic queue requiered by the application
      \param desiredDeviceFeatures the device feature requiered by the application
      \return a reference to a ComputeQueue
      */
      void initDevices(
        int nbComputeQueue,
        int nbGraphicQueue,
        VkPhysicalDeviceFeatures* desiredDeviceFeatures = nullptr){
        HeadlessInitialisator headless;
        initDevices(nbComputeQueue, nbGraphicQueue, headless, true, desiredDeviceFeatures);
      }

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
      \param optional: a boolean indicating if the device creation can success even if raytracing features are not found
      */
      void enableRaytracing(bool optional = false);

      /**
      \brief Ask the device to enable all feature related to mesh shaders
      \param optional: a boolean indicating if the device creation can success even if mesh shader features are not found
      */
      void enableMeshShader(bool optional = false);

      /*
      \brief check if raytracing features have been successfuly loaded
      \return true if raytracing is available
      */
      bool raytracingAvailable() const {
        return m_raytracingAvailable;
      }

      /*
      \brief check if mesh shader features have been successfuly loaded
      \return true if mesh shader is available
      */
      bool meshShaderAvailable() const {
        return m_meshShaderAvailable;
      }

    private:

      void initDevices(
          int nbComputeQueue,
          int nbGraphicQueue,
          SurfaceInitialisator& window,
          bool headless,
          VkPhysicalDeviceFeatures* desiredDeviceFeatures = nullptr);

      

	  VkPhysicalDevice                                      m_physical = VK_NULL_HANDLE;
	  VkDevice                                              m_logical = VK_NULL_HANDLE;
	  LIBRARY_TYPE                                          m_vulkanLibrary = nullptr;
	  VkInstance                                            m_instance = VK_NULL_HANDLE;
	  VkSurfaceKHR                                          m_presentationSurface = VK_NULL_HANDLE;
	  VkCommandPool                                         m_commandPool = VK_NULL_HANDLE;
	  std::vector<GraphicQueue>	                            m_graphicQueues;
	  std::vector<ComputeQueue>                             m_computeQueues;
	  PresentationQueue                                     m_presentQueue;


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
