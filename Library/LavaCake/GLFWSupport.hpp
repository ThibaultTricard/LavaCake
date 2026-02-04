#pragma once

/**
 * \file GLFWSupport.hpp
 * \brief Optional GLFW convenience utilities for LavaCake
 *
 * Include this header only if you want to use GLFW as your windowing library.
 * This header depends on GLFW3 being available.
 */

#include "Device.hpp"
#include <GLFW/glfw3.h>

namespace LavaCake {
namespace GLFW {

    /**
     * \brief Create a SurfaceConfig for use with a GLFW window
     *
     * This is a convenience function that handles getting the required
     * Vulkan instance extensions from GLFW and creating the surface creation callback.
     *
     * Example usage:
     * \code
     * glfwInit();
     * glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
     * GLFWwindow* window = glfwCreateWindow(800, 600, "My App", nullptr, nullptr);
     *
     * auto config = LavaCake::GLFW::createSurfaceConfig(window);
     * LavaCake::Device device(config, 1);  // 1 graphics queue
     * \endcode
     *
     * \param window The GLFW window to create a Vulkan surface for
     * \return SurfaceConfig ready to pass to the Device constructor
     *
     * \warning The window pointer is captured by the callback. Ensure the window
     *          remains valid until after the Device is constructed.
     */
    inline SurfaceConfig createSurfaceConfig(GLFWwindow* window) {
        SurfaceConfig config;

        // Get required instance extensions from GLFW
        uint32_t glfwExtCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);
        if (glfwExtensions) {
            config.requiredExtensions.assign(glfwExtensions, glfwExtensions + glfwExtCount);
        }

        // Snapshot the current framebuffer size so the swapchain has a valid
        // fallback on platforms that report currentExtent as UINT32_MAX
        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        config.width  = static_cast<uint32_t>(fbWidth);
        config.height = static_cast<uint32_t>(fbHeight);

        // Create surface callback that captures the window pointer
        config.createSurface = [window](vk::Instance instance) -> vk::SurfaceKHR {
            VkSurfaceKHR rawSurface;
            if (glfwCreateWindowSurface(static_cast<VkInstance>(instance),
                                        window, nullptr, &rawSurface) != VK_SUCCESS) {
                return vk::SurfaceKHR();
            }
            return vk::SurfaceKHR(rawSurface);
        };

        return config;
    }

} // namespace GLFW
} // namespace LavaCake
