#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_extension_inspection.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <optional>
#include <set>
#include <map>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


#include "SwapChainImage.hpp"

uint32_t api_version = VK_API_VERSION_1_3;

namespace LavaCake {

    // ---------------------------------------------------------------
    // Debug Callback
    // ---------------------------------------------------------------

    /**
     * \brief Debug callback function for Vulkan validation layers (macOS version)
     * \param severity the severity level of the message
     * \param type the type of message
     * \param data the callback data containing the message
     * \param userData optional user data pointer
     * \return VK_FALSE to continue execution
     */
#ifdef __APPLE__
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* data,
        void* userData)
    {
        std::cerr << "Validation: " << data->pMessage << std::endl;
        return VK_FALSE;
    }
#else
    /**
     * \brief Debug callback function for Vulkan validation layers
     * \param messageSeverity the severity level of the message
     * \param messageType the type of message
     * \param data the callback data containing the message
     * \param pUserData optional user data pointer
     * \return VK_FALSE to continue execution
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* pUserData)
{
        if(data->messageIdNumber != 0)
        std::cerr << "Validation: " << data->messageIdNumber  << " Message: "<< data->pMessage << std::endl;
        return VK_FALSE;
}
#endif

    // ---------------------------------------------------------------
    // Device scoring: pick the most powerful GPU
    // ---------------------------------------------------------------
    /**
     * \brief Scores a physical device to determine the most powerful GPU
     * \param device the physical device to score
     * \return the score value, higher is better
     */
    int scoreDevice(vk::PhysicalDevice device)
    {
        auto props = device.getProperties();
        auto memProps = device.getMemoryProperties();

        int score = 0;

        // 1) Most important: discrete > integrated > others
        if      (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)   score += 100000;
        else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) score += 10000;
        else if (props.deviceType == vk::PhysicalDeviceType::eVirtualGpu)    score += 1000;
        else if (props.deviceType == vk::PhysicalDeviceType::eCpu)           score += 100;

        // 2) More VRAM = more points
        // Sum all heaps for simplicity (not all are device-local)
        for (const auto& heap : memProps.memoryHeaps) {
            score += static_cast<int>(heap.size / (1024 * 1024));
        }

        return score;
    }

    /**
    \brief helps manage Vulkan device related task
    */
    class Device {
    public:

        Device(){};

        /**
         * \brief Copy constructor, copies a device
         * \param d the device to copy
         */
        Device(const Device& d){
            m_physicalDevice = d.m_physicalDevice;
            m_device = d.m_device;
            m_instance = d.m_instance ;
            m_presentationSurface = d.m_presentationSurface;
            m_swapchain = d.m_swapchain;
            m_swapchainImages = d.m_swapchainImages;
            m_swapchainFormat = d.m_swapchainFormat;
            m_swapchainExtent = d.m_swapchainExtent;
            m_commandPool = d.m_commandPool;
            m_debugMessenger = d.m_debugMessenger;
            m_graphicQueues = d.m_graphicQueues;
            m_computeQueues = d.m_computeQueues;
            m_presentQueue = d.m_presentQueue;
            m_hasSurface = d.m_hasSurface;
            m_allocator = d.m_allocator;
        };

        /**
         * \brief Copy assignment operator, copies a device
         * \param d the device to copy
         * \return reference to this device
         */
        Device& operator=(const Device& d) {
            m_physicalDevice = d.m_physicalDevice;
            m_device = d.m_device;
            m_instance = d.m_instance ;
            m_presentationSurface = d.m_presentationSurface;
            m_swapchain = d.m_swapchain;
            m_swapchainImages = d.m_swapchainImages;
            m_swapchainFormat = d.m_swapchainFormat;
            m_swapchainExtent = d.m_swapchainExtent;
            m_commandPool = d.m_commandPool;
            m_debugMessenger = d.m_debugMessenger;
            m_graphicQueues = d.m_graphicQueues;
            m_computeQueues = d.m_computeQueues;
            m_presentQueue = d.m_presentQueue;
            m_hasSurface = d.m_hasSurface;
            m_allocator = d.m_allocator;
            return *this;
        };

        /**
         * \brief Move constructor, moves a device
         * \param d the device to move
         */
        Device(Device&& d) noexcept{
            m_physicalDevice = d.m_physicalDevice;
            m_device = d.m_device;
            m_instance = d.m_instance ;
            m_presentationSurface = d.m_presentationSurface;
            m_swapchain = d.m_swapchain;
            m_swapchainImages = d.m_swapchainImages;
            m_swapchainFormat = d.m_swapchainFormat;
            m_swapchainExtent = d.m_swapchainExtent;
            m_commandPool = d.m_commandPool;
            m_debugMessenger = d.m_debugMessenger;
            m_graphicQueues = d.m_graphicQueues;
            m_computeQueues = d.m_computeQueues;
            m_presentQueue = d.m_presentQueue;
            m_hasSurface = d.m_hasSurface;
            m_allocator = d.m_allocator;
        }

        /**
         * \brief Move assignment operator, moves a device
         * \param d the device to move
         * \return reference to this device
         */
        Device& operator=(Device&& d) noexcept
        {
            m_physicalDevice = d.m_physicalDevice;
            m_device = d.m_device;
            m_instance = d.m_instance ;
            m_presentationSurface = d.m_presentationSurface;
            m_swapchain = d.m_swapchain;
            m_swapchainImages = d.m_swapchainImages;
            m_swapchainFormat = d.m_swapchainFormat;
            m_swapchainExtent = d.m_swapchainExtent;
            m_commandPool = d.m_commandPool;
            m_debugMessenger = d.m_debugMessenger;
            m_graphicQueues = d.m_graphicQueues;
            m_computeQueues = d.m_computeQueues;
            m_presentQueue = d.m_presentQueue;
            m_hasSurface = d.m_hasSurface;
            m_allocator = d.m_allocator;
            return *this;
        }

        
        /**
        * \brief Return the Vulkan instance
        * \return the vk::Instance used by the application
        */
        const vk::Instance& getInstance() const {
            return m_instance;
        };

        /**
        * \brief Return the Device
        * \return the vk::Device used by the application
        */
        const vk::Device& getDevice() const{
            return m_device;
        }


        /**
        * \brief Return the Physical Device
        * \return the vk::PhysicalDevice used by the application
        */
        const vk::PhysicalDevice& getPhysicalDevice() const{
            return m_physicalDevice;
        }



        /**
        * \brief Return the Command pool
        * \return the vk::CommandPool used by the application
        */
        const vk::CommandPool& getCommandPool() const{
            return m_commandPool;
        }

        /**
        * \brief Return the Vulkan surface
        * \return the vk::SurfaceKHR used by the application
        */
        const vk::SurfaceKHR& getSurface() const{
            return m_presentationSurface;
        }

        /**
        * \brief Return the Presentation Queue used to draw on the screen
        * \return a reference to the vk::Queue used by the application
        */
        const vk::Queue& getPresentQueue() const{
            return m_presentQueue;
        }

        /**
        * \brief Return a specific Graphic Queue
        * \param i the index of the wanted queue
        * \return a reference to a vk::Queue
        */
        const vk::Queue& getGraphicQueue(int i) const{
            return m_graphicQueues[i];
        }

        /**
        * \brief Return a specific Compute Queue
        * \param i the index of the wanted queue
        * \return a reference to a vk::Queue
        */
        const vk::Queue& getComputeQueue(int i)const{
            return m_computeQueues[i];
        }

        /**
        * \brief Return the first Queue available 
        * a graphic queue if it exists,
        * a compute queue if it exists,
        * a present queue else
        * \return a reference to a vk::Queue
        */
        const vk::Queue& getAnyQueue()const{
            if(m_graphicQueues.size() > 0) return m_graphicQueues[0];
            if(m_computeQueues.size() > 0) return m_computeQueues[0];
            return m_presentQueue;
        }


        /**
        * \brief Return the Vulkan Memory Allocator
        * \return the Vulkan Memory Allocatoused by the application
        */
        const VmaAllocator& getAllocator() const {
            return m_allocator;
        };


        /**
        * \brief Initialise the device with a surface
        * \param window the window to use for the surface creation
        * \param nbComputeQueue the number of compute queue requiered by the application
        * \param nbGraphicQueue the number of graphic queue requiered by the application
        */
        Device(
            GLFWwindow* window ,
            int nbGraphicQueue,
            int nbComputeQueue = 0
            ) : Device(nbGraphicQueue,nbComputeQueue,true,window){

            }
        
        /**
        * \brief Initialise the device without a surface
        * \param nbComputeQueue the number of compute queue requiered by the application
        * \param nbGraphicQueue the number of graphic queue requiered by the application
        */
        Device(
            int nbGraphicQueue,
            int nbComputeQueue = 1
            ) : Device(nbGraphicQueue,nbComputeQueue,false,nullptr){

            }

        /**
        * \brief Make sure every command send to the device are finished
        */
        void waitForAllCommands(){
            m_device.waitIdle();
        }

        /**
        * \brief Allocates and returns a Command Buffer 
        * the command buffer is the reponsability of the calling function,
        * it will not be destroyed by the device
        * \return a vk::CommandBuffer
        */

        vk::CommandBuffer allocateCommandBuffer() const{
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_commandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = 1;
            return m_device.allocateCommandBuffers(allocInfo)[0];
        }

        /**
        * \brief Allocates and returns multiple Command Buffers
        * the command buffers are the responsibility of the calling function,
        * they will not be destroyed by the device
        * \param number the number of command buffers to allocate
        * \return a std::vector of vk::CommandBuffer
        */
        std::vector<vk::CommandBuffer> allocateCommandBuffers(uint32_t number){
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_commandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = number;
            return m_device.allocateCommandBuffers(allocInfo);
        }

        /**
        * \brief Frees a Command Buffer 
        * \param cmd the command buffer to free
        */
        void freeCommandBuffer(vk::CommandBuffer cmd) const{
            m_device.freeCommandBuffers(m_commandPool, cmd);
        }

        /**
        * \brief Frees multiple Command Buffers
        * \param cmd the vector of command buffers to free
        */
        void freeCommandBuffers(std::vector<vk::CommandBuffer> cmd) const{
            m_device.freeCommandBuffers(m_commandPool, cmd);
        }



        /**
         * \brief Acquires the next available swapchain image
         * \param isAvailableSemaphore the semaphore to signal when the image is available
         * \return reference to the acquired SwapChainImage
         */
        LavaCake::SwapChainImage& aquireSwapChainImage(vk::Semaphore isAvailableSemaphore){
            uint32_t imageIndex;

            vk::Result result = m_device.acquireNextImageKHR(
                m_swapchain,
                UINT64_MAX,
                isAvailableSemaphore,
                nullptr,
                &imageIndex
            );

            return m_swapchainImages[imageIndex];
        }

        /**
         * \brief Returns the swapchain image format
         * \return the vk::Format of the swapchain
         */
        vk::Format getSwapchainFormat(){
            return m_swapchainFormat;
        }

        /**
         * \brief Returns the swapchain Extent
         * \return the vk::Extent2D of the swapchain
         */
        vk::Extent2D getSwapchainExtent(){
            return m_swapchainExtent;
        }

        /**
         * \brief Returns the number of swapchain Image
         * \return the size_t of the swapchain Images array
         */ 
        size_t getSwapChainImagesNumber(){
           return m_swapchainImages.size(); 
        }

        /**
         * \brief Presents a swapchain image to the screen
         * \param image the SwapChainImage to present
         * \param semaphores the semaphores to wait on before presenting
         * \return vk::Result indicating success or failure
         */
        vk::Result presentImage(LavaCake::SwapChainImage image, std::vector<vk::Semaphore>semaphores){
            uint32_t index = image.getIndex();
            vk::PresentInfoKHR present{};
            present.waitSemaphoreCount = semaphores.size();
            present.pWaitSemaphores = semaphores.data();
            present.swapchainCount = 1;
            present.pSwapchains = &m_swapchain;
            present.pImageIndices = &index;

            return getPresentQueue().presentKHR(present);
        }

        /**
         * \brief Destroy the device
         */
        void releaseDevice() {
            vmaDestroyAllocator(m_allocator);
            m_device.destroyCommandPool(m_commandPool);
            if(m_hasSurface)  {
                m_device.destroySwapchainKHR(m_swapchain);
                //for(auto v : m_swapchainImagesView) m_device.destroyImageView(v);
            }
            m_device.destroy();
            if(m_hasSurface)  {
                m_instance.destroySurfaceKHR(m_presentationSurface);
            }
            m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
            m_instance.destroy();
        }

    private:
        vk::PhysicalDevice                                    m_physicalDevice;
        vk::Device                                            m_device;
        vk::Instance                                          m_instance;
        vk::SurfaceKHR                                        m_presentationSurface;

        vk::SwapchainKHR                                      m_swapchain;
        std::vector<LavaCake::SwapChainImage>                 m_swapchainImages;
        vk::Format                                            m_swapchainFormat;
        vk::Extent2D                                          m_swapchainExtent;

        vk::CommandPool                                       m_commandPool;
        vk::DebugUtilsMessengerEXT                            m_debugMessenger;
        std::vector<vk::Queue>	                              m_graphicQueues;
        std::vector<vk::Queue>                                m_computeQueues;
        vk::Queue                                             m_presentQueue;

        bool                                                  m_hasSurface;

        VmaAllocator                                          m_allocator;





        
        /**
        * \brief Initialise the device
        * \param nbComputeQueue the number of compute queue requiered by the application
        * \param nbGraphicQueue the number of graphic queue requiered by the application
        * \param createSurface true if the device needs to create a surface
        * \param window the window to use for the surface creation
        * \return a reference to a ComputeQueue
        */
        Device(
            int nbGraphicQueue,
            int nbComputeQueue,
            bool createSurface,
            GLFWwindow* window
            ){

            try
            {
                // -----------------------------------------------------------
                // 2) Create Vulkan Instance
                // -----------------------------------------------------------
                VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

                std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };

                uint32_t glfwExtCount = 0;
                const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);
                std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtCount);
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                #ifdef __APPLE__
                    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                #endif

                auto instanceExtensionProps = vk::enumerateInstanceExtensionProperties();
            

                for (const vk::ExtensionProperties& ext : instanceExtensionProps) {
                    if (vk::isDeprecatedExtension(ext.extensionName)) continue;
                    if (vk::isPromotedExtension(ext.extensionName)) continue;
                    if (vk::isObsoletedExtension(ext.extensionName)) continue;
                    instanceExtensions.push_back(ext.extensionName);
                }



                vk::ApplicationInfo appInfo{
                    "Vulkan Full Example",
                    VK_MAKE_VERSION(1,0,0),
                    "Lavacake",
                    VK_MAKE_VERSION(2,0,0),
                    api_version
                };

                vk::InstanceCreateInfo instanceInfo{};
                instanceInfo.pApplicationInfo = &appInfo;
                instanceInfo.enabledLayerCount = layers.size();
                instanceInfo.ppEnabledLayerNames = layers.data();
                instanceInfo.enabledExtensionCount = instanceExtensions.size();
                instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

                

                #ifdef __APPLE__
                instanceInfo.flags |=
                    vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
                #endif
                
                m_instance = vk::createInstance(instanceInfo);
                VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

                // -----------------------------------------------------------
                // 3) Create Debug Messenger
                // -----------------------------------------------------------
                vk::DebugUtilsMessengerCreateInfoEXT debugInfo{};
                debugInfo.messageSeverity =
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

                debugInfo.messageType =
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

                debugInfo.pfnUserCallback = debugCallback;

                m_debugMessenger =
                    m_instance.createDebugUtilsMessengerEXT(debugInfo);

                // -----------------------------------------------------------
                // 4) Create Window Surface
                // -----------------------------------------------------------
                if(createSurface){
                    VkSurfaceKHR rawSurface;
                    glfwCreateWindowSurface(static_cast<VkInstance>(m_instance), window, nullptr, &rawSurface);
                    m_presentationSurface = vk::SurfaceKHR (rawSurface);
                }

                // -----------------------------------------------------------
                // 5) Enumerate and select the best physical device
                // -----------------------------------------------------------
                auto devices = m_instance.enumeratePhysicalDevices();
                if (devices.empty()) throw std::runtime_error("No Vulkan devices found.");

                m_physicalDevice = VK_NULL_HANDLE;
                int bestScore = -1;

                for (auto& dev : devices)
                {
                    int score = scoreDevice(dev);
                    if (score > bestScore)
                    {
                        bestScore = score;
                        m_physicalDevice = dev;
                    }
                }

                auto props = m_physicalDevice.getProperties();
                //std::cout << "Selected GPU: " << props.deviceName << "\n";

                // -----------------------------------------------------------
                // 6) Find Graphics + Present Queue Families
                // -----------------------------------------------------------
                auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();

                std::optional<uint32_t> graphicsFamily;
                std::optional<uint32_t> presentFamily;
                std::optional<uint32_t> computeFamily;

                bool graphicsFamilyTooSmall = false;
                bool computeFamilyTooSmall = false;

                for (uint32_t i = 0; i < queueFamilies.size(); i++)
                {
                    if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics){
                        if(queueFamilies[i].queueCount >= nbGraphicQueue){
                            graphicsFamily = i;
                            graphicsFamilyTooSmall = false;
                        }
                        else{
                            graphicsFamilyTooSmall = true;
                        }
                    }

                    if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute){
                        if(queueFamilies[i].queueCount >= nbComputeQueue){
                            computeFamily = i;
                            computeFamilyTooSmall = false;
                        }
                        else{
                            computeFamilyTooSmall = true;
                        }
                    }

                    if (createSurface && m_physicalDevice.getSurfaceSupportKHR(i, m_presentationSurface))
                        presentFamily = i;

                    if ((graphicsFamily || nbGraphicQueue ==0 ) && (presentFamily || !createSurface) && (computeFamily || nbComputeQueue == 0)) 
                        break;
                }
                

                if(graphicsFamilyTooSmall){
                    throw std::invalid_argument( "The requested number of graphics queue is higher than the number available on this device" );
                }

                if(computeFamilyTooSmall){
                    throw std::invalid_argument( "The requested number of compute queue is higher than the number available on this device" );
                }
                /*TODO 
                * refaire ce check
                */
                //if (!graphicsFamily || !presentFamily)
                //    throw std::runtime_error("Could not find required queue families.");

                // -----------------------------------------------------------
                // 7) Enable ALL available device extensions
                // -----------------------------------------------------------
                auto availableExts = m_physicalDevice.enumerateDeviceExtensionProperties();
                std::vector<const char*> deviceExtensions;

                for (auto& ext : availableExts){
                    if (vk::isDeprecatedExtension(ext.extensionName)) { continue;}
                    if (vk::isPromotedExtension(ext.extensionName)) { continue;}
                    if (vk::isObsoletedExtension(ext.extensionName)) { continue;}
                    if (std::string(ext.extensionName) == "VK_AMD_shader_fragment_mask") continue;
                    if (std::string(ext.extensionName) == "VK_NV_shading_rate_image") continue;
                    
                    if (std::string(ext.extensionName) == "VK_EXT_descriptor_buffer") continue;
                    if (std::string(ext.extensionName) == "VK_NV_disk_cache_utils") continue;

                    if (std::string(ext.extensionName) == "VK_NV_internal_nvpresent") continue;
                    
                    deviceExtensions.push_back(ext.extensionName);
                }

                // -----------------------------------------------------------
                // 8) Create Logical Device
                // -----------------------------------------------------------
                float queuePriority = 1.0f;

                std::vector<vk::DeviceQueueCreateInfo> queueInfos;

                
                
                bool gaphicQueueCreated = false;
                bool computeQueueCreated = false;

                //Graphique Queues
                if(nbGraphicQueue > 0){
                    gaphicQueueCreated = true;
                    uint32_t nbQueue = nbGraphicQueue;
                    if(graphicsFamily.value() == computeFamily.value()){
                        computeQueueCreated = true;
                        nbQueue = nbQueue< nbComputeQueue? nbComputeQueue: nbQueue;
                    }
                    vk::DeviceQueueCreateInfo gqinfo{};
                    gqinfo.queueFamilyIndex = graphicsFamily.value();
                    gqinfo.queueCount = nbQueue;
                    gqinfo.pQueuePriorities = &queuePriority;
                    queueInfos.push_back(gqinfo);
                }

                //Compute Queues
                if(nbComputeQueue > 0 && !computeQueueCreated){
                    computeQueueCreated = true;
                    vk::DeviceQueueCreateInfo cqinfo{};
                    cqinfo.queueFamilyIndex = computeFamily.value();
                    cqinfo.queueCount = nbComputeQueue;
                    cqinfo.pQueuePriorities = &queuePriority;
                    queueInfos.push_back(cqinfo);
                }

                //Present Queues
                if(createSurface){
                    
                    if( gaphicQueueCreated && graphicsFamily.value() == presentFamily.value()){
                        //present queue has allready been created
                    }
                    else if( computeQueueCreated && computeFamily.value() == presentFamily.value()){
                        //present queue has allready been created
                    }
                    else {
                        vk::DeviceQueueCreateInfo pqinfo{};
                        pqinfo.queueFamilyIndex = presentFamily.value();
                        pqinfo.queueCount = 1;
                        pqinfo.pQueuePriorities = &queuePriority;
                        queueInfos.push_back(pqinfo);
                    }
                    
                }

                vk::PhysicalDeviceFeatures features{};

                vk::DeviceCreateInfo devInfo{};
                devInfo.queueCreateInfoCount = queueInfos.size();
                devInfo.pQueueCreateInfos = queueInfos.data();
                devInfo.pEnabledFeatures = &features;
                devInfo.enabledExtensionCount = deviceExtensions.size();
                devInfo.ppEnabledExtensionNames = deviceExtensions.data();

                vk::PhysicalDeviceVulkan12Features vulkan12Features{};
                vulkan12Features.scalarBlockLayout = VK_TRUE;

                vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
                dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
                dynamicRenderingFeatures.pNext = &vulkan12Features;

                devInfo.pNext = &dynamicRenderingFeatures;

                m_device = m_physicalDevice.createDevice(devInfo);
                VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);

                if(nbGraphicQueue > 0){
                    for(int i = 0; i< nbGraphicQueue; i++){
                        m_graphicQueues.push_back(m_device.getQueue(graphicsFamily.value(), i));
                    }
                }
                if(nbComputeQueue > 0){
                    for(int i = 0; i< nbComputeQueue; i++){
                        m_computeQueues.push_back(m_device.getQueue(computeFamily.value(), i));
                    }
                }
                
                if(createSurface){
                    m_presentQueue  = m_device.getQueue(presentFamily.value(), 0);
                }


                // -----------------------------------------------------------
                // 9) Create Swapchain
                // -----------------------------------------------------------
                if(createSurface){
                    auto surfaceCaps = m_physicalDevice.getSurfaceCapabilitiesKHR(m_presentationSurface);
                    auto formats = m_physicalDevice.getSurfaceFormatsKHR(m_presentationSurface);
                    auto presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_presentationSurface);

                    vk::SurfaceFormatKHR surfaceFormat = formats[0];
                    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

                    m_swapchainExtent = surfaceCaps.currentExtent;

                    uint32_t imageCount = surfaceCaps.minImageCount + 1;
                    if (surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount)
                        imageCount = surfaceCaps.maxImageCount;

                    vk::SwapchainCreateInfoKHR swapInfo{};
                    swapInfo.surface = m_presentationSurface;
                    swapInfo.minImageCount = imageCount;
                    swapInfo.imageFormat = surfaceFormat.format;
                    swapInfo.imageColorSpace = surfaceFormat.colorSpace;
                    swapInfo.imageExtent = m_swapchainExtent;
                    swapInfo.imageArrayLayers = 1;
                    swapInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

                    uint32_t indices[] = {
                        graphicsFamily.value(),
                        presentFamily.value()
                    };

                    if (graphicsFamily != presentFamily)
                    {
                        swapInfo.imageSharingMode = vk::SharingMode::eConcurrent;
                        swapInfo.queueFamilyIndexCount = 2;
                        swapInfo.pQueueFamilyIndices = indices;
                    }
                    else
                    {
                        swapInfo.imageSharingMode = vk::SharingMode::eExclusive;
                    }

                    swapInfo.preTransform = surfaceCaps.currentTransform;
                    swapInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
                    swapInfo.presentMode = presentMode;
                    swapInfo.clipped = VK_TRUE;

                    m_swapchainFormat = surfaceFormat.format;

                    m_swapchain = m_device.createSwapchainKHR(swapInfo);

                    auto images = m_device.getSwapchainImagesKHR(m_swapchain);
                    m_swapchainImages.reserve(images.size());

                    for(uint32_t i = 0; i< images.size(); i ++){ 
                        SwapChainImage swapChainImage(m_device, images[i], i ,m_swapchainFormat);
                        m_swapchainImages.push_back(swapChainImage);
                    }
                    
                   

                    //std::cout << "Swapchain created with " << m_swapchainImages.size() << " images.\n";
                }  

                // -----------------------------------------------------------
                // 10) Command Pool
                // -----------------------------------------------------------
                vk::CommandPoolCreateInfo poolInfo{};
                poolInfo.queueFamilyIndex = graphicsFamily.value();
                poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

                m_commandPool = m_device.createCommandPool(poolInfo);

                if(createSurface){
                    m_hasSurface = true;
                }


                // -----------------------------------------------------------
                // 11) VMA
                // -----------------------------------------------------------

                VmaAllocatorCreateInfo allocatorInfo{};
                allocatorInfo.instance = m_instance;
                allocatorInfo.physicalDevice = m_physicalDevice;
                allocatorInfo.device = m_device;
                allocatorInfo.vulkanApiVersion = api_version;

                // Optional flags (enable only if extensions are enabled)
                allocatorInfo.flags = 0;
                // allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

                if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create VMA allocator");
                }

                //std::cout << "VMA allocator created.\n";


            }
            catch (std::exception& e)
            {
                std::cerr << "ERROR: " << e.what() << "\n";
            }
        }

    };
}