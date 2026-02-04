#pragma once

/**
 * \file SDL2Support.hpp
 * \brief Optional SDL2 convenience utilities for LavaCake
 *
 * Include this header only if you want to use SDL2 as your windowing library.
 * This header depends on SDL2 being available.
 */

#include "Device.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace LavaCake {
namespace SDL2 {

    /**
     * \brief Create a SurfaceConfig for use with an SDL2 window
     *
     * This is a convenience function that handles getting the required
     * Vulkan instance extensions from SDL2 and creating the surface creation callback.
     *
     * Example usage:
     * \code
     * SDL_Init(SDL_INIT_VIDEO);
     * SDL_Window* window = SDL_CreateWindow("My App",
     *     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
     *     800, 600, SDL_WINDOW_VULKAN);
     *
     * auto config = LavaCake::SDL2::createSurfaceConfig(window);
     * LavaCake::Device device(config, 1);  // 1 graphics queue
     * \endcode
     *
     * \param window The SDL2 window to create a Vulkan surface for
     * \return SurfaceConfig ready to pass to the Device constructor
     *
     * \warning The window pointer is captured by the callback. Ensure the window
     *          remains valid until after the Device is constructed.
     */
    inline SurfaceConfig createSurfaceConfig(SDL_Window* window) {
        SurfaceConfig config;

        // Get required instance extensions from SDL2
        unsigned int extCount = 0;
        SDL_Vulkan_GetInstanceExtensions(window, &extCount, nullptr);
        if (extCount > 0) {
            config.requiredExtensions.resize(extCount);
            SDL_Vulkan_GetInstanceExtensions(window, &extCount, config.requiredExtensions.data());
        }

        // Snapshot the current window size so the swapchain has a valid
        // fallback on platforms that report currentExtent as UINT32_MAX
        int sdlWidth = 0, sdlHeight = 0;
        SDL_Vulkan_GetWindowSize(window, &sdlWidth, &sdlHeight);
        config.width  = static_cast<uint32_t>(sdlWidth);
        config.height = static_cast<uint32_t>(sdlHeight);

        // Create surface callback that captures the window pointer
        config.createSurface = [window](vk::Instance instance) -> vk::SurfaceKHR {
            VkSurfaceKHR rawSurface;
            if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), &rawSurface)) {
                return vk::SurfaceKHR();
            }
            return vk::SurfaceKHR(rawSurface);
        };

        return config;
    }

} // namespace SDL2
} // namespace LavaCake
