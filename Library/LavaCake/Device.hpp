#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_extension_inspection.hpp>
#include <iostream>
#include <optional>
#include <set>
#include <map>
#include <any>
#include <functional>

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
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* pUserData)
{
        if(data->messageIdNumber == 0 || data->messageIdNumber == 2044605652) return VK_FALSE;
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

    // ---------------------------------------------------------------
    // Surface Configuration (for window-manager agnostic design)
    // ---------------------------------------------------------------

    /**
     * \brief Callback type for creating a VkSurfaceKHR from a VkInstance
     *
     * Users implement this using their windowing library's surface creation API.
     * Return a null/empty SurfaceKHR on failure.
     */
    using SurfaceCreateFn = std::function<vk::SurfaceKHR(vk::Instance instance)>;

    /**
     * \brief Configuration for creating a Device with a presentation surface
     *
     * This struct decouples LavaCake from any specific windowing library.
     * Users provide the required Vulkan instance extensions and a callback
     * to create the surface using their windowing system of choice.
     *
     * Example with SDL2:
     * \code
     * SurfaceConfig config;
     * unsigned int count;
     * SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
     * config.requiredExtensions.resize(count);
     * SDL_Vulkan_GetInstanceExtensions(window, &count, config.requiredExtensions.data());
     * config.createSurface = [window](vk::Instance inst) -> vk::SurfaceKHR {
     *     VkSurfaceKHR surf;
     *     SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(inst), &surf);
     *     return vk::SurfaceKHR(surf);
     * };
     * \endcode
     */
    struct SurfaceConfig {
        /**
         * \brief Required Vulkan instance extensions for surface support
         *
         * These are window-system-specific extensions needed for presentation.
         * Typically includes VK_KHR_surface plus platform-specific extensions
         * (e.g., VK_KHR_win32_surface, VK_KHR_xcb_surface, VK_MVK_macos_surface, etc.)
         */
        std::vector<const char*> requiredExtensions;

        /**
         * \brief Callback function to create the VkSurfaceKHR
         *
         * This function will be called after the Vulkan instance is created.
         * The user implements this using their windowing library's surface creation API.
         * Return a null SurfaceKHR on failure.
         */
        SurfaceCreateFn createSurface;
    };

    /**
    \brief helps manage Vulkan device related task
    */
    class Device {
    public:

        // Forward declaration of nested builder class
        class Builder;

        Device()
            : m_physicalDevice(VK_NULL_HANDLE)
            , m_device(VK_NULL_HANDLE)
            , m_instance(VK_NULL_HANDLE)
            , m_presentationSurface(VK_NULL_HANDLE)
            , m_swapchain(VK_NULL_HANDLE)
            , m_swapchainFormat(vk::Format::eUndefined)
            , m_commandPool(VK_NULL_HANDLE)
            , m_debugMessenger(VK_NULL_HANDLE)
            , m_hasSurface(false)
            , m_allocator(VK_NULL_HANDLE)
        {};

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
        * \brief Make sure every command send to the device are finished
        */
        void waitForAllCommands(){
            m_device.waitIdle();
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
                // Destroy swapchain image views (owned by SwapChainImage objects)
                for(auto& img : m_swapchainImages) {
                    m_device.destroyImageView(img.getImageView());
                }
                m_swapchainImages.clear();
                m_device.destroySwapchainKHR(m_swapchain);
            }
            m_device.destroy();
            if(m_hasSurface)  {
                m_instance.destroySurfaceKHR(m_presentationSurface);
            }
            if(m_debugMessenger) {
                m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
            }
            m_instance.destroy();
        }


        /**
         * \brief Implicit conversion to vk::Device
         * \return the vk::Device handle
         */
        operator vk::Device() const{
            return m_device;
        }

        // ---------------------------------------------------------------
        // Nested Builder Class
        // ---------------------------------------------------------------

        /**
         * \brief Fluent builder for configuring and creating Device instances
         *
         * The Builder class provides a fluent API for constructing Vulkan devices with
         * custom configurations. It follows the builder pattern, allowing method chaining
         * to configure queues, features, extensions, and other device properties.
         *
         * Key features:
         * - Fluent API with method chaining
         * - Support for headless and windowed devices
         * - Generic feature chain management for any Vulkan feature structure
         * - Mandatory dynamic rendering support
         * - Configurable validation layers
         * - Custom device selection
         *
         * Example usage:
         * \code
         * auto device = Device::Builder()
         *     .setGraphicQueues(2)
         *     .setComputeQueues(1)
         *     .enableSamplerAnisotropy(true)
         *     .preferDiscreteGPU()
         *     .build();
         * \endcode
         */
        class Builder {
        private:
            /**
             * \brief Type-erased wrapper for storing Vulkan feature structures
             *
             * This struct uses std::any to store copies of any Vulkan feature structure,
             * along with function pointers to access and manipulate the pNext chain.
             * This allows the builder to support any Vulkan feature type without
             * requiring smart pointers or dynamic allocation beyond std::any.
             */
            struct FeatureWrapper {
                std::any feature;  ///< Stores the actual feature structure by value
                void* (*getPtr)(std::any&);  ///< Function pointer to get feature address
                void (*setPNext)(std::any&, void*);  ///< Function pointer to set pNext
            };

        public:
            /**
             * \brief Constructs a Builder with default settings
             *
             * Initializes the builder with:
             * - Dynamic rendering enabled (mandatory, cannot be disabled)
             * - Default validation layers enabled
             * - Default device selector (prefers discrete GPU)
             */
            Builder() {
                // Dynamic rendering is mandatory and always enabled
                m_dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
            }

            // ---------------------------------------------------------------
            // Queue Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Sets the number of graphics queues to create
             * \param count Number of graphics queues (must be supported by the physical device)
             * \return Reference to this builder for method chaining
             */
            Builder& setGraphicQueueCount(int count) {
                m_nbGraphicQueues = count;
                return *this;
            }

            /**
             * \brief Sets the number of compute queues to create
             * \param count Number of compute queues (must be supported by the physical device)
             * \return Reference to this builder for method chaining
             */
            Builder& setComputeQueueCount(int count) {
                m_nbComputeQueues = count;
                return *this;
            }

            // ---------------------------------------------------------------
            // Surface Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Configures the device for windowed rendering with a presentation surface
             * \param config Surface configuration containing required extensions and surface creation callback
             * \return Reference to this builder for method chaining
             * \details This method enables presentation support and configures the swapchain.
             *          The surface configuration is window-system agnostic and must be provided
             *          by the user using their windowing library (e.g., SDL, GLFW).
             */
            Builder& setSurface(const SurfaceConfig& config) {
                m_surfaceConfig = config;
                m_hasSurface = true;
                return *this;
            }

            /**
             * \brief Configures the device for headless operation (no presentation surface)
             * \return Reference to this builder for method chaining
             * \details Headless devices are suitable for compute workloads or offscreen rendering
             *          without a window or display output.
             */
            Builder& headless() {
                m_hasSurface = false;
                m_surfaceConfig = std::nullopt;
                return *this;
            }

            // ---------------------------------------------------------------
            // API Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Sets the Vulkan API version to request
             * \param version Vulkan API version (e.g., VK_API_VERSION_1_3)
             * \return Reference to this builder for method chaining
             */
            Builder& setApiVersion(uint32_t version) {
                m_apiVersion = version;
                return *this;
            }

            /**
             * \brief Sets application and engine information for the Vulkan instance
             * \param appName Application name
             * \param appVersion Application version
             * \param engineName Engine name (defaults to "LavaCake")
             * \param engineVersion Engine version (defaults to 2.0.0)
             * \return Reference to this builder for method chaining
             */
            Builder& setApplicationInfo(
                const char* appName,
                uint32_t appVersion,
                const char* engineName = "LavaCake",
                uint32_t engineVersion = VK_MAKE_VERSION(2, 0, 0)
            ) {
                m_appName = appName;
                m_appVersion = appVersion;
                m_engineName = engineName;
                m_engineVersion = engineVersion;
                return *this;
            }

            // ---------------------------------------------------------------
            // Validation Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Enables or disables Vulkan validation layers
             * \param enable True to enable validation, false to disable
             * \return Reference to this builder for method chaining
             * \details Validation layers are enabled by default in debug builds
             *          and should be disabled in release builds for better performance.
             */
            Builder& enableValidation(bool enable = true) {
                m_enableValidation = enable;
                return *this;
            }

            /**
             * \brief Sets custom validation layers
             * \param layers Vector of validation layer names
             * \return Reference to this builder for method chaining
             */
            Builder& setValidationLayers(std::vector<const char*> layers) {
                m_validationLayers = std::move(layers);
                return *this;
            }

            // ---------------------------------------------------------------
            // Instance Extension Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Adds a single instance extension
             * \param extension Extension name (e.g., VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
             * \return Reference to this builder for method chaining
             */
            Builder& addInstanceExtension(const char* extension) {
                m_instanceExtensions.push_back(extension);
                return *this;
            }

            /**
             * \brief Adds multiple instance extensions
             * \param extensions Vector of extension names
             * \return Reference to this builder for method chaining
             */
            Builder& addInstanceExtensions(std::vector<const char*> extensions) {
                m_instanceExtensions.insert(
                    m_instanceExtensions.end(),
                    extensions.begin(),
                    extensions.end()
                );
                return *this;
            }

            // ---------------------------------------------------------------
            // Device Extension Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Adds a single device extension
             * \param extension Extension name (e.g., VK_KHR_SWAPCHAIN_EXTENSION_NAME)
             * \return Reference to this builder for method chaining
             * \note The swapchain extension is automatically added when a surface is configured
             */
            Builder& addDeviceExtension(const char* extension) {
                m_deviceExtensions.push_back(extension);
                return *this;
            }

            /**
             * \brief Adds multiple device extensions
             * \param extensions Vector of extension names
             * \return Reference to this builder for method chaining
             */
            Builder& addDeviceExtensions(std::vector<const char*> extensions) {
                m_deviceExtensions.insert(
                    m_deviceExtensions.end(),
                    extensions.begin(),
                    extensions.end()
                );
                return *this;
            }

            // ---------------------------------------------------------------
            // Physical Device Selection
            // ---------------------------------------------------------------

            /**
             * \brief Sets a custom device selector function
             * \param selector Function that scores physical devices (higher score = preferred)
             * \return Reference to this builder for method chaining
             * \details The selector function receives a vk::PhysicalDevice and returns
             *          an integer score. The device with the highest score will be selected.
             */
            Builder& setDeviceSelector(std::function<int(vk::PhysicalDevice)> selector) {
                m_deviceSelector = std::move(selector);
                return *this;
            }

            /**
             * \brief Configures the builder to prefer discrete GPUs over integrated GPUs
             * \return Reference to this builder for method chaining
             * \details This is the default behavior. Discrete GPUs receive higher scores
             *          than integrated GPUs in the device selection process.
             */
            Builder& preferDiscreteGPU() {
                m_deviceSelector = scoreDevice;
                return *this;
            }

            // ---------------------------------------------------------------
            // Feature Configuration
            // ---------------------------------------------------------------
            //
            // Individual feature setters for vk::PhysicalDeviceFeatures
            // Each method enables or disables a specific Vulkan 1.0 feature
            // All methods return a reference to the builder for method chaining
            //
            // Example usage:
            //   .enableGeometryShader(true)
            //   .enableTessellationShader(true)
            //   .enableSamplerAnisotropy(true)
            // ---------------------------------------------------------------

            /**
             * \brief Enables or disables robust buffer access
             * \param enable True to enable, false to disable
             * \return Reference to this builder for method chaining
             */
            Builder& enableRobustBufferAccess(bool enable = true) {
                m_features.robustBufferAccess = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableFullDrawIndexUint32(bool enable = true) {
                m_features.fullDrawIndexUint32 = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableImageCubeArray(bool enable = true) {
                m_features.imageCubeArray = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableIndependentBlend(bool enable = true) {
                m_features.independentBlend = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableGeometryShader(bool enable = true) {
                m_features.geometryShader = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableTessellationShader(bool enable = true) {
                m_features.tessellationShader = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSampleRateShading(bool enable = true) {
                m_features.sampleRateShading = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableDualSrcBlend(bool enable = true) {
                m_features.dualSrcBlend = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableLogicOp(bool enable = true) {
                m_features.logicOp = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableMultiDrawIndirect(bool enable = true) {
                m_features.multiDrawIndirect = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableDrawIndirectFirstInstance(bool enable = true) {
                m_features.drawIndirectFirstInstance = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableDepthClamp(bool enable = true) {
                m_features.depthClamp = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableDepthBiasClamp(bool enable = true) {
                m_features.depthBiasClamp = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableFillModeNonSolid(bool enable = true) {
                m_features.fillModeNonSolid = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableDepthBounds(bool enable = true) {
                m_features.depthBounds = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableWideLines(bool enable = true) {
                m_features.wideLines = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableLargePoints(bool enable = true) {
                m_features.largePoints = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableAlphaToOne(bool enable = true) {
                m_features.alphaToOne = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableMultiViewport(bool enable = true) {
                m_features.multiViewport = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSamplerAnisotropy(bool enable = true) {
                m_features.samplerAnisotropy = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableTextureCompressionETC2(bool enable = true) {
                m_features.textureCompressionETC2 = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableTextureCompressionASTC_LDR(bool enable = true) {
                m_features.textureCompressionASTC_LDR = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableTextureCompressionBC(bool enable = true) {
                m_features.textureCompressionBC = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableOcclusionQueryPrecise(bool enable = true) {
                m_features.occlusionQueryPrecise = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enablePipelineStatisticsQuery(bool enable = true) {
                m_features.pipelineStatisticsQuery = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableVertexPipelineStoresAndAtomics(bool enable = true) {
                m_features.vertexPipelineStoresAndAtomics = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableFragmentStoresAndAtomics(bool enable = true) {
                m_features.fragmentStoresAndAtomics = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderTessellationAndGeometryPointSize(bool enable = true) {
                m_features.shaderTessellationAndGeometryPointSize = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderImageGatherExtended(bool enable = true) {
                m_features.shaderImageGatherExtended = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderStorageImageExtendedFormats(bool enable = true) {
                m_features.shaderStorageImageExtendedFormats = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderStorageImageMultisample(bool enable = true) {
                m_features.shaderStorageImageMultisample = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderStorageImageReadWithoutFormat(bool enable = true) {
                m_features.shaderStorageImageReadWithoutFormat = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderStorageImageWriteWithoutFormat(bool enable = true) {
                m_features.shaderStorageImageWriteWithoutFormat = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderUniformBufferArrayDynamicIndexing(bool enable = true) {
                m_features.shaderUniformBufferArrayDynamicIndexing = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderSampledImageArrayDynamicIndexing(bool enable = true) {
                m_features.shaderSampledImageArrayDynamicIndexing = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderStorageBufferArrayDynamicIndexing(bool enable = true) {
                m_features.shaderStorageBufferArrayDynamicIndexing = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderStorageImageArrayDynamicIndexing(bool enable = true) {
                m_features.shaderStorageImageArrayDynamicIndexing = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderClipDistance(bool enable = true) {
                m_features.shaderClipDistance = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderCullDistance(bool enable = true) {
                m_features.shaderCullDistance = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderFloat64(bool enable = true) {
                m_features.shaderFloat64 = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderInt64(bool enable = true) {
                m_features.shaderInt64 = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderInt16(bool enable = true) {
                m_features.shaderInt16 = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderResourceResidency(bool enable = true) {
                m_features.shaderResourceResidency = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableShaderResourceMinLod(bool enable = true) {
                m_features.shaderResourceMinLod = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseBinding(bool enable = true) {
                m_features.sparseBinding = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidencyBuffer(bool enable = true) {
                m_features.sparseResidencyBuffer = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidencyImage2D(bool enable = true) {
                m_features.sparseResidencyImage2D = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidencyImage3D(bool enable = true) {
                m_features.sparseResidencyImage3D = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidency2Samples(bool enable = true) {
                m_features.sparseResidency2Samples = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidency4Samples(bool enable = true) {
                m_features.sparseResidency4Samples = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidency8Samples(bool enable = true) {
                m_features.sparseResidency8Samples = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidency16Samples(bool enable = true) {
                m_features.sparseResidency16Samples = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableSparseResidencyAliased(bool enable = true) {
                m_features.sparseResidencyAliased = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableVariableMultisampleRate(bool enable = true) {
                m_features.variableMultisampleRate = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            Builder& enableInheritedQueries(bool enable = true) {
                m_features.inheritedQueries = enable ? VK_TRUE : VK_FALSE;
                return *this;
            }

            // ---------------------------------------------------------------
            // Generic Feature Chain Management
            // ---------------------------------------------------------------

            /**
             * \brief Adds a Vulkan feature structure to the device feature chain
             * \tparam FeatureStruct Any Vulkan feature structure type (e.g., vk::PhysicalDeviceVulkan12Features)
             * \param features The feature structure to add
             * \return Reference to this builder for method chaining
             *
             * \details This template method supports ANY Vulkan feature structure that has
             *          a pNext member. The feature is stored by value and automatically
             *          linked into the pNext chain during device creation. Multiple features
             *          can be added and will be chained together in the order they were added.
             *
             * Supported feature types include (but are not limited to):
             * - vk::PhysicalDeviceVulkan11Features
             * - vk::PhysicalDeviceVulkan12Features
             * - vk::PhysicalDeviceVulkan13Features
             * - vk::PhysicalDeviceRayTracingPipelineFeaturesKHR
             * - vk::PhysicalDeviceMeshShaderFeaturesEXT
             * - Any other Vulkan feature structure
             *
             * Example usage:
             * \code
             * vk::PhysicalDeviceVulkan12Features vulkan12{};
             * vulkan12.bufferDeviceAddress = VK_TRUE;
             * vulkan12.descriptorIndexing = VK_TRUE;
             *
             * builder.addFeature(vulkan12);
             * \endcode
             */
            template<typename FeatureStruct>
            Builder& addFeature(const FeatureStruct& features) {
                FeatureWrapper wrapper;
                wrapper.feature = features;

                // Set function pointers for type-specific operations
                wrapper.getPtr = [](std::any& f) -> void* {
                    return &std::any_cast<FeatureStruct&>(f);
                };

                wrapper.setPNext = [](std::any& f, void* next) {
                    std::any_cast<FeatureStruct&>(f).pNext = next;
                };

                m_customFeatures.push_back(std::move(wrapper));
                return *this;
            }

            // ---------------------------------------------------------------
            // VMA Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Sets Vulkan Memory Allocator (VMA) creation flags
             * \param flags VMA allocator creation flags
             * \return Reference to this builder for method chaining
             */
            Builder& setVmaFlags(VmaAllocatorCreateFlags flags) {
                m_vmaFlags = flags;
                return *this;
            }

            // ---------------------------------------------------------------
            // Swapchain Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Sets the preferred number of swapchain images
             * \param count Number of swapchain images
             * \return Reference to this builder for method chaining
             * \note Only applies when a surface is configured
             */
            Builder& setSwapchainImageCount(uint32_t count) {
                m_preferredSwapchainImageCount = count;
                return *this;
            }

            /**
             * \brief Sets the preferred swapchain present mode
             * \param mode Present mode (e.g., vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo)
             * \return Reference to this builder for method chaining
             * \note Only applies when a surface is configured
             */
            Builder& setPresentMode(vk::PresentModeKHR mode) {
                m_preferredPresentMode = mode;
                return *this;
            }

            /**
             * \brief Sets the preferred swapchain surface format
             * \param format Surface format
             * \return Reference to this builder for method chaining
             * \note Only applies when a surface is configured
             */
            Builder& setSurfaceFormat(vk::SurfaceFormatKHR format) {
                m_preferredSurfaceFormat = format;
                return *this;
            }

            // ---------------------------------------------------------------
            // Build
            // ---------------------------------------------------------------

            /**
             * \brief Constructs and returns the configured Device
             * \return Fully configured Device instance
             * \throws std::runtime_error if validation fails or device creation fails
             *
             * \details This method performs the following operations:
             * 1. Validates the builder configuration
             * 2. Creates the Vulkan instance with requested extensions
             * 3. Sets up debug messenger if validation is enabled
             * 4. Creates presentation surface if configured
             * 5. Selects physical device using the configured selector
             * 6. Finds appropriate queue families
             * 7. Builds the feature pNext chain from all added features
             * 8. Creates the logical device
             * 9. Retrieves queue handles
             * 10. Creates swapchain if surface is present
             * 11. Creates command pool
             * 12. Initializes Vulkan Memory Allocator (VMA)
             *
             * Requirements:
             * - At least one graphics or compute queue must be requested
             * - Surface configuration must be provided if hasSurface is true
             * - Physical device must support requested queue counts
             */
            Device build() {
                // Validate configuration
                if (m_nbGraphicQueues == 0 && m_nbComputeQueues == 0) {
                    throw std::runtime_error("At least one graphic or compute queue is required");
                }

                if (m_hasSurface && !m_surfaceConfig.has_value()) {
                    throw std::runtime_error("Surface configuration required when hasSurface is true");
                }

                Device device;

                try
                {
                    // Prepare extensions
                    std::vector<const char*> additionalExtensions;
                    SurfaceCreateFn surfaceCreator = nullptr;

                    if (m_hasSurface && m_surfaceConfig.has_value()) {
                        additionalExtensions = m_surfaceConfig->requiredExtensions;
                        surfaceCreator = m_surfaceConfig->createSurface;
                    }

                    // Add user-specified instance extensions
                    additionalExtensions.insert(
                        additionalExtensions.end(),
                        m_instanceExtensions.begin(),
                        m_instanceExtensions.end()
                    );

                    // -----------------------------------------------------------
                    // Create Vulkan Instance
                    // -----------------------------------------------------------
                    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

                    // Use validation layers from builder
                    std::vector<const char*> layers;
                    if (m_enableValidation) {
                        layers = m_validationLayers;
                    }

                    // Start with user-provided extensions
                    std::vector<const char*> instanceExtensions(additionalExtensions.begin(), additionalExtensions.end());
                    if (m_enableValidation) {
                        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    }

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

                    // Use application info from builder
                    vk::ApplicationInfo appInfo{
                        m_appName,
                        m_appVersion,
                        m_engineName,
                        m_engineVersion,
                        m_apiVersion
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

                    device.m_instance = vk::createInstance(instanceInfo);
                    VULKAN_HPP_DEFAULT_DISPATCHER.init(device.m_instance);

                    // -----------------------------------------------------------
                    // Create Debug Messenger
                    // -----------------------------------------------------------
                    if (m_enableValidation) {
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

                        device.m_debugMessenger =
                            device.m_instance.createDebugUtilsMessengerEXT(debugInfo);
                    }

                    // -----------------------------------------------------------
                    // Create Window Surface
                    // -----------------------------------------------------------
                    if(m_hasSurface && surfaceCreator){
                        device.m_presentationSurface = surfaceCreator(device.m_instance);
                        if(!device.m_presentationSurface){
                            throw std::runtime_error("Failed to create presentation surface");
                        }
                    }

                    // -----------------------------------------------------------
                    // Enumerate and select physical device
                    // -----------------------------------------------------------
                    auto devices = device.m_instance.enumeratePhysicalDevices();
                    if (devices.empty()) throw std::runtime_error("No Vulkan devices found.");

                    device.m_physicalDevice = VK_NULL_HANDLE;
                    int bestScore = -1;

                    // Use device selector from builder
                    for (auto& dev : devices)
                    {
                        int score = m_deviceSelector(dev);
                        if (score > bestScore)
                        {
                            bestScore = score;
                            device.m_physicalDevice = dev;
                        }
                    }

                    // -----------------------------------------------------------
                    // Find Queue Families
                    // -----------------------------------------------------------
                    auto queueFamilies = device.m_physicalDevice.getQueueFamilyProperties();

                    std::optional<uint32_t> graphicsFamily;
                    std::optional<uint32_t> presentFamily;
                    std::optional<uint32_t> computeFamily;

                    bool graphicsFamilyTooSmall = false;
                    bool computeFamilyTooSmall = false;

                    for (uint32_t i = 0; i < queueFamilies.size(); i++)
                    {
                        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics){
                            if(queueFamilies[i].queueCount >= m_nbGraphicQueues){
                                graphicsFamily = i;
                                graphicsFamilyTooSmall = false;
                            }
                            else{
                                graphicsFamilyTooSmall = true;
                            }
                        }

                        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute){
                            if(queueFamilies[i].queueCount >= m_nbComputeQueues){
                                computeFamily = i;
                                computeFamilyTooSmall = false;
                            }
                            else{
                                computeFamilyTooSmall = true;
                            }
                        }

                        if (m_hasSurface && device.m_physicalDevice.getSurfaceSupportKHR(i, device.m_presentationSurface))
                            presentFamily = i;

                        if ((graphicsFamily || m_nbGraphicQueues ==0 ) && (presentFamily || !m_hasSurface) && (computeFamily || m_nbComputeQueues == 0))
                            break;
                    }

                    if(graphicsFamilyTooSmall){
                        throw std::invalid_argument( "The requested number of graphics queue is higher than the number available on this device" );
                    }

                    if(computeFamilyTooSmall){
                        throw std::invalid_argument( "The requested number of compute queue is higher than the number available on this device" );
                    }

                    // -----------------------------------------------------------
                    // Setup device extensions
                    // -----------------------------------------------------------
                    std::vector<const char*> deviceExtensions = m_deviceExtensions;

                    // Automatically add swapchain extension if surface is present
                    if (m_hasSurface) {
                        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                    }

                    // -----------------------------------------------------------
                    // Create Logical Device with builder features
                    // -----------------------------------------------------------
                    float queuePriority = 1.0f;
                    std::vector<vk::DeviceQueueCreateInfo> queueInfos;

                    bool gaphicQueueCreated = false;
                    bool computeQueueCreated = false;

                    //Graphique Queues
                    if(m_nbGraphicQueues > 0){
                        gaphicQueueCreated = true;
                        uint32_t nbQueue = m_nbGraphicQueues;
                        if(graphicsFamily.value() == computeFamily.value()){
                            computeQueueCreated = true;
                            nbQueue = nbQueue< m_nbComputeQueues? m_nbComputeQueues: nbQueue;
                        }
                        vk::DeviceQueueCreateInfo gqinfo{};
                        gqinfo.queueFamilyIndex = graphicsFamily.value();
                        gqinfo.queueCount = nbQueue;
                        gqinfo.pQueuePriorities = &queuePriority;
                        queueInfos.push_back(gqinfo);
                    }

                    //Compute Queues
                    if(m_nbComputeQueues > 0 && !computeQueueCreated){
                        computeQueueCreated = true;
                        vk::DeviceQueueCreateInfo cqinfo{};
                        cqinfo.queueFamilyIndex = computeFamily.value();
                        cqinfo.queueCount = m_nbComputeQueues;
                        cqinfo.pQueuePriorities = &queuePriority;
                        queueInfos.push_back(cqinfo);
                    }

                    //Present Queues
                    if(m_hasSurface){
                        if( gaphicQueueCreated && graphicsFamily.value() == presentFamily.value()){
                            //present queue has already been created
                        }
                        else if( computeQueueCreated && computeFamily.value() == presentFamily.value()){
                            //present queue has already been created
                        }
                        else {
                            vk::DeviceQueueCreateInfo pqinfo{};
                            pqinfo.queueFamilyIndex = presentFamily.value();
                            pqinfo.queueCount = 1;
                            pqinfo.pQueuePriorities = &queuePriority;
                            queueInfos.push_back(pqinfo);
                        }
                    }

                    // Use features from builder
                    vk::DeviceCreateInfo devInfo{};
                    devInfo.queueCreateInfoCount = queueInfos.size();
                    devInfo.pQueueCreateInfos = queueInfos.data();
                    devInfo.pEnabledFeatures = &m_features;
                    devInfo.enabledExtensionCount = deviceExtensions.size();
                    devInfo.ppEnabledExtensionNames = deviceExtensions.data();

                    // Build feature chain: Chain all custom features together
                    void* featureChainTail = nullptr;

                    // Chain custom features in reverse order (last added becomes deepest in chain)
                    for (auto it = m_customFeatures.rbegin(); it != m_customFeatures.rend(); ++it) {
                        auto& wrapper = *it;
                        wrapper.setPNext(wrapper.feature, featureChainTail);
                        featureChainTail = wrapper.getPtr(wrapper.feature);
                    }

                    // Dynamic rendering (mandatory) is always at the head of the chain
                    m_dynamicRenderingFeatures.pNext = featureChainTail;
                    devInfo.pNext = &m_dynamicRenderingFeatures;

                    device.m_device = device.m_physicalDevice.createDevice(devInfo);
                    VULKAN_HPP_DEFAULT_DISPATCHER.init(device.m_device);

                    // Get queues
                    if(m_nbGraphicQueues > 0){
                        for(int i = 0; i< m_nbGraphicQueues; i++){
                            device.m_graphicQueues.push_back(device.m_device.getQueue(graphicsFamily.value(), i));
                        }
                    }
                    if(m_nbComputeQueues > 0){
                        for(int i = 0; i< m_nbComputeQueues; i++){
                            device.m_computeQueues.push_back(device.m_device.getQueue(computeFamily.value(), i));
                        }
                    }

                    if(m_hasSurface){
                        device.m_presentQueue  = device.m_device.getQueue(presentFamily.value(), 0);
                    }

                    // -----------------------------------------------------------
                    // Create Swapchain (use builder preferences if provided)
                    // -----------------------------------------------------------
                    if(m_hasSurface){
                        auto surfaceCaps = device.m_physicalDevice.getSurfaceCapabilitiesKHR(device.m_presentationSurface);
                        auto formats = device.m_physicalDevice.getSurfaceFormatsKHR(device.m_presentationSurface);
                        auto presentModes = device.m_physicalDevice.getSurfacePresentModesKHR(device.m_presentationSurface);

                        // Use preferred surface format if provided, otherwise use first available
                        vk::SurfaceFormatKHR surfaceFormat = m_preferredSurfaceFormat.value_or(formats[0]);

                        // Use preferred present mode if provided, otherwise use FIFO
                        vk::PresentModeKHR presentMode = m_preferredPresentMode.value_or(vk::PresentModeKHR::eFifo);

                        device.m_swapchainExtent = surfaceCaps.currentExtent;

                        // Use preferred image count if provided
                        uint32_t imageCount = m_preferredSwapchainImageCount.value_or(surfaceCaps.minImageCount + 1);
                        if (surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount)
                            imageCount = surfaceCaps.maxImageCount;

                        vk::SwapchainCreateInfoKHR swapInfo{};
                        swapInfo.surface = device.m_presentationSurface;
                        swapInfo.minImageCount = imageCount;
                        swapInfo.imageFormat = surfaceFormat.format;
                        swapInfo.imageColorSpace = surfaceFormat.colorSpace;
                        swapInfo.imageExtent = device.m_swapchainExtent;
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

                        device.m_swapchainFormat = surfaceFormat.format;

                        device.m_swapchain = device.m_device.createSwapchainKHR(swapInfo);

                        auto images = device.m_device.getSwapchainImagesKHR(device.m_swapchain);
                        device.m_swapchainImages.reserve(images.size());

                        for(uint32_t i = 0; i< images.size(); i ++){
                            SwapChainImage swapChainImage(device.m_device, images[i], i ,device.m_swapchainFormat);
                            device.m_swapchainImages.push_back(swapChainImage);
                        }
                    }

                    // -----------------------------------------------------------
                    // Command Pool
                    // -----------------------------------------------------------
                    vk::CommandPoolCreateInfo poolInfo{};
                    poolInfo.queueFamilyIndex = graphicsFamily.value();
                    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

                    device.m_commandPool = device.m_device.createCommandPool(poolInfo);

                    
                    device.m_hasSurface = m_hasSurface;
                    

                    // -----------------------------------------------------------
                    // VMA with builder flags
                    // -----------------------------------------------------------
                    VmaAllocatorCreateInfo allocatorInfo{};
                    allocatorInfo.instance = device.m_instance;
                    allocatorInfo.physicalDevice = device.m_physicalDevice;
                    allocatorInfo.device = device.m_device;
                    allocatorInfo.vulkanApiVersion = m_apiVersion;
                    allocatorInfo.flags = m_vmaFlags;

                    if (vmaCreateAllocator(&allocatorInfo, &device.m_allocator) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create VMA allocator");
                    }
                }
                catch (std::exception& e)
                {
                    std::cerr << "ERROR: " << e.what() << "\n";
                }

                return device;
            }

        private:
            // Queue configuration
            int m_nbGraphicQueues = 1;
            int m_nbComputeQueues = 0;

            // Surface configuration
            bool m_hasSurface = false;
            std::optional<SurfaceConfig> m_surfaceConfig;

            // API version
            uint32_t m_apiVersion = VK_API_VERSION_1_3;

            // Validation
            bool m_enableValidation = true;
            std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

            // Extensions
            std::vector<const char*> m_instanceExtensions;
            std::vector<const char*> m_deviceExtensions;

            // Application info
            const char* m_appName = "";
            uint32_t m_appVersion = VK_MAKE_VERSION(1, 0, 0);
            const char* m_engineName = "LavaCake";
            uint32_t m_engineVersion = VK_MAKE_VERSION(2, 0, 0);

            // Device selection
            std::function<int(vk::PhysicalDevice)> m_deviceSelector = scoreDevice;

            // Features
            vk::PhysicalDeviceFeatures m_features{};

            // Dynamic rendering feature (mandatory - always enabled)
            vk::PhysicalDeviceDynamicRenderingFeatures m_dynamicRenderingFeatures{};

            // Generic feature chain storage
            std::vector<FeatureWrapper> m_customFeatures;

            // VMA
            VmaAllocatorCreateFlags m_vmaFlags = 0;

            // Swapchain preferences
            std::optional<uint32_t> m_preferredSwapchainImageCount;
            std::optional<vk::PresentModeKHR> m_preferredPresentMode;
            std::optional<vk::SurfaceFormatKHR> m_preferredSurfaceFormat;
        };

    private:
        friend class Builder;

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






    };

    // ---------------------------------------------------------------
    // Device Factory Functions
    // ---------------------------------------------------------------

    /**
     * \brief Creates a basic headless graphics device with minimal configuration
     * \return Device configured for basic graphics usage
     *
     * \details Creates a headless device with:
     * - 1 graphics queue
     * - 0 compute queues
     * - No presentation surface
     * - Dynamic rendering enabled (mandatory)
     * - Default validation layers enabled
     *
     * This is the simplest device configuration, suitable for basic rendering tasks
     * or learning purposes.
     */
    inline Device createBasicDevice() {
        return Device::Builder()
            .setGraphicQueueCount(1)
            .setComputeQueueCount(0)
            .headless()
            .build();
    }

    /**
     * \brief Creates an advanced graphics device with enhanced features
     * \param surfaceConfig Surface configuration for presentation
     * \param nbGraphicQueue Number of graphics queues
     * \param nbComputeQueue Number of compute queues
     * \return Device configured for advanced graphics usage
     *
     * \details Creates a windowed device with:
     * - Custom queue configuration
     * - Anisotropic filtering enabled
     * - Descriptor indexing enabled (Vulkan 1.2 features)
     * - Scalar block layout
     * - Runtime descriptor arrays
     * - Dynamic rendering enabled (mandatory)
     * - Discrete GPU preference
     * - Presentation surface support
     *
     * This configuration is suitable for modern graphics applications requiring
     * advanced Vulkan 1.2+ features.
     */
    inline Device createAdvancedDevice(const SurfaceConfig& surfaceConfig, int nbGraphicQueue, int nbComputeQueue = 0) {
        // Setup Vulkan 1.2 features for descriptor indexing
        vk::PhysicalDeviceVulkan12Features vulkan12{};
        vulkan12.scalarBlockLayout = VK_TRUE;
        vulkan12.descriptorIndexing = VK_TRUE;
        vulkan12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        vulkan12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        vulkan12.runtimeDescriptorArray = VK_TRUE;
        vulkan12.descriptorBindingPartiallyBound = VK_TRUE;
        vulkan12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        vulkan12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;

        return Device::Builder()
            .setGraphicQueueCount(nbGraphicQueue)
            .setComputeQueueCount(nbComputeQueue)
            .enableSamplerAnisotropy(true)
            .addFeature(vulkan12)
            .preferDiscreteGPU()
            .setSurface(surfaceConfig)
            .build();
    }

    /**
     * \brief Creates a headless device for compute or offscreen rendering
     * \param nbGraphicQueue Number of graphics queues (default: 1)
     * \param nbComputeQueue Number of compute queues (default: 0)
     * \return Device configured for headless usage
     *
     * \details Creates a headless device with:
     * - Custom queue configuration
     * - No presentation surface
     * - Dynamic rendering enabled (mandatory)
     * - Default validation layers enabled
     *
     * Headless devices are ideal for:
     * - Compute-only workloads
     * - Offscreen rendering
     * - Server-side graphics processing
     * - Automated testing
     */
    inline Device createHeadlessDevice(int nbGraphicQueue = 1, int nbComputeQueue = 0) {
        return Device::Builder()
            .setGraphicQueueCount(nbGraphicQueue)
            .setComputeQueueCount(nbComputeQueue)
            .headless()
            .build();
    }

    /**
     * \brief Creates a windowed device with presentation support
     * \param surfaceConfig Surface configuration for window presentation
     * \param nbGraphicQueue Number of graphics queues (default: 1)
     * \param nbComputeQueue Number of compute queues (default: 0)
     * \return Device configured for windowed usage
     *
     * \details Creates a windowed device with:
     * - Custom queue configuration
     * - Presentation surface support
     * - Swapchain creation
     * - Dynamic rendering enabled (mandatory)
     * - Default validation layers enabled
     *
     * The surface configuration must be provided using your windowing library
     * (e.g., SDL2, GLFW). The device will automatically enable the swapchain
     * extension and create a swapchain for presentation.
     */
    inline Device createWindowedDevice(
        const SurfaceConfig& surfaceConfig,
        int nbGraphicQueue = 1,
        int nbComputeQueue = 0
    ) {
        return Device::Builder()
            .setGraphicQueueCount(nbGraphicQueue)
            .setComputeQueueCount(nbComputeQueue)
            .setSurface(surfaceConfig)
            .build();
    }

    /**
     * \brief Returns a new device builder for custom configuration
     * \return Builder instance for fluent configuration
     *
     * \details Use this function to access the builder for advanced device
     *          configuration with full control over all options.
     *
     * Example usage:
     * \code
     * auto device = createDeviceBuilder()
     *     .setGraphicQueues(2)
     *     .setComputeQueues(1)
     *     .enableGeometryShader(true)
     *     .enableTessellationShader(true)
     *     .preferDiscreteGPU()
     *     .build();
     * \endcode
     */
    inline Device::Builder createDeviceBuilder() {
        return Device::Builder();
    }

}