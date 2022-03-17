#include "SwapChain.h"
namespace LavaCake {
  namespace Framework {

    //SwapChain* SwapChain::m_swapChain;

    void SwapChain::init(
      VkImageUsageFlags													swapchain_image_usage
    ) {
      Device* d = Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      VkPhysicalDevice physical = d->getPhysicalDevice();
      VkSurfaceKHR surface = d->getSurface();

      m_swapchainImageUsage = swapchain_image_usage;

      VkResult result = vkDeviceWaitIdle(logical);
      if (result != VK_SUCCESS) {
        ErrorCheck::setError("Waiting on a device failed.");
      }

      VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
      VkPresentModeKHR present_mode;
      //========================Select Presentation Mode

      // Enumerate supported present modes
      uint32_t present_modes_count = 0;

      result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &present_modes_count, nullptr);
      if ((result != VK_SUCCESS) ||
        (present_modes_count == 0)) {
        ErrorCheck::setError("Could not get the number of supported present modes.");
      }

      std::vector<VkPresentModeKHR> present_modes(present_modes_count);
      result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &present_modes_count, present_modes.data());
      if ((result != VK_SUCCESS) ||
        (present_modes_count == 0)) {
        ErrorCheck::setError("Could not enumerate present modes.");
      }

      // Select present mode
      bool found = false;
      for (auto& current_present_mode : present_modes) {
        if (current_present_mode == desired_present_mode) {
          present_mode = desired_present_mode;
          found = true;
        }
      }
      if (!found) {
        ErrorCheck::setError("Desired present mode is not supported. Selecting default FIFO mode.", 1);
        for (auto& current_present_mode : present_modes) {
          if (current_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
            present_mode = VK_PRESENT_MODE_FIFO_KHR;
            found = true;
          }
        }
      }
      if (!found) {
        ErrorCheck::setError("VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory for all drivers!");
        return;
      }

      //========================

      VkSurfaceCapabilitiesKHR surface_capabilities;
      result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &surface_capabilities);

      if (VK_SUCCESS != result)ErrorCheck::setError("Could not get surface capabilities");

      uint32_t number_of_images;
      number_of_images = surface_capabilities.minImageCount + 1;
      if ((surface_capabilities.maxImageCount > 0) &&
        (number_of_images > surface_capabilities.maxImageCount)) {
        number_of_images = surface_capabilities.maxImageCount;
      }


      //========================Size of the swapchain
      if (0xFFFFFFFF == surface_capabilities.currentExtent.width) {
        m_size = { 640, 480 };

        if (m_size.width < surface_capabilities.minImageExtent.width) {
          m_size.width = surface_capabilities.minImageExtent.width;
        }
        else if (m_size.width > surface_capabilities.maxImageExtent.width) {
          m_size.width = surface_capabilities.maxImageExtent.width;
        }

        if (m_size.height < surface_capabilities.minImageExtent.height) {
          m_size.height = surface_capabilities.minImageExtent.height;
        }
        else if (m_size.height > surface_capabilities.maxImageExtent.height) {
          m_size.height = surface_capabilities.maxImageExtent.height;
        }
      }
      else {
        m_size = surface_capabilities.currentExtent;
      }

      if ((0 == m_size.width) ||
        (0 == m_size.height)) {
        ErrorCheck::setError("Wrong swapchain image size");
      }

      //========================Image usage
      VkImageUsageFlags image_usage;
      image_usage = swapchain_image_usage & surface_capabilities.supportedUsageFlags;

      if (swapchain_image_usage != image_usage) {
        ErrorCheck::setError("Could not select swapchain image image usage");
      }

      //========================Surface Transform
      VkSurfaceTransformFlagBitsKHR surface_transform;
      if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
      }
      else {
        surface_transform = surface_capabilities.currentTransform;
      }

      //========================Swap Chain format
      VkColorSpaceKHR image_color_space;
      VkSurfaceFormatKHR desired_surface_format = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

      uint32_t formats_count = 0;

      result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &formats_count, nullptr);
      if ((VK_SUCCESS != result) ||
        (0 == formats_count)) {
        ErrorCheck::setError("Could not get the number of supported surface formats.");
      }

      std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
      result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &formats_count, surface_formats.data());
      if ((VK_SUCCESS != result) ||
        (0 == formats_count)) {
        ErrorCheck::setError("Could not enumerate supported surface formats.");
      }

      // Select surface format
      found = false;
      if ((1 == surface_formats.size()) &&
        (VK_FORMAT_UNDEFINED == surface_formats[0].format)) {
        m_format = desired_surface_format.format;
        image_color_space = desired_surface_format.colorSpace;
        found = true;
      }

      if (!found) {
        for (auto& surface_format : surface_formats) {
          if ((desired_surface_format.format == surface_format.format) &&
            (desired_surface_format.colorSpace == surface_format.colorSpace)) {
            m_format = desired_surface_format.format;
            image_color_space = desired_surface_format.colorSpace;
            found = true;
          }
        }
      }
      if (!found) {
        for (auto& surface_format : surface_formats) {
          if (desired_surface_format.format == surface_format.format) {
            m_format = desired_surface_format.format;
            image_color_space = surface_format.colorSpace;
            ErrorCheck::setError("Desired combination of format and colorspace is not supported. Selecting other colorspace.", 1);
            found = true;
          }
        }
      }

      if (!found) {
        m_format = surface_formats[0].format;
        image_color_space = surface_formats[0].colorSpace;
        ErrorCheck::setError("Desired format is not supported. Selecting available format - colorspace combination.", 1);
      }

      //======================== Creating SwapChain

      VkSwapchainCreateInfoKHR swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType                  sType
        nullptr,                                      // const void                     * pNext
        0,                                            // VkSwapchainCreateFlagsKHR        flags
        surface,                                      // VkSurfaceKHR                     surface
        number_of_images,                             // uint32_t                         minImageCount
        m_format,                                     // VkFormat                         imageFormat
        image_color_space,                            // VkColorSpaceKHR                  imageColorSpace
        m_size,                                       // VkExtent2D                       imageExtent
        1,                                            // uint32_t                         imageArrayLayers
        image_usage,                                  // VkImageUsageFlags                imageUsage
        VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode                    imageSharingMode
        0,                                            // uint32_t                         queueFamilyIndexCount
        nullptr,                                      // const uint32_t                 * pQueueFamilyIndices
        surface_transform,                            // VkSurfaceTransformFlagBitsKHR    preTransform
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR      compositeAlpha
        present_mode,                                 // VkPresentModeKHR                 presentMode
        VK_TRUE,                                      // VkBool32                         clipped
        VK_NULL_HANDLE                                 // VkSwapchainKHR                   oldSwapchain
      };

      result = vkCreateSwapchainKHR(logical, &swapchain_create_info, nullptr, &m_handle);
      if ((result != VK_SUCCESS) ||
        (m_handle == VK_NULL_HANDLE)) {
        ErrorCheck::setError("Could not create a swapchain.");
        return;
      }

      //======================== SwapChain Image handles
      uint32_t images_count = 0;
      std::vector<VkImage> swapchainImages;
      /*if (!LavaCake::Core::GetHandlesOfSwapchainImages(logical, m_handle, swapchainImages)) {
        ErrorCheck::setError("Could not get handle of the swapchain images");
      }*/
      result = vkGetSwapchainImagesKHR(logical, m_handle, &images_count, nullptr);
      if ((VK_SUCCESS != result) ||
        (0 == images_count)) {
        ErrorCheck::setError("Could not get the number of swapchain images.");
        return;
      }

      swapchainImages.resize(images_count);
      result = vkGetSwapchainImagesKHR(logical, m_handle, &images_count, swapchainImages.data());
      if ((result != VK_SUCCESS) ||
        (images_count == 0)) {
        ErrorCheck::setError("Could not enumerate swapchain images.");
        return;
      }

      for (uint32_t i = 0; i < swapchainImages.size(); ++i) {
        m_swapchainImages.push_back(new SwapChainImage(m_size, m_format, swapchainImages[i]));
      }
    }

    void SwapChain::resize() {

      Device* d = Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      vkDestroySwapchainKHR(logical, m_handle, nullptr);
      for (uint32_t i = 0; i < m_swapchainImages.size(); i++) {
        delete m_swapchainImages[i];
      }
      m_swapchainImages.clear();
      init(m_swapchainImageUsage);
    }

  }
}
