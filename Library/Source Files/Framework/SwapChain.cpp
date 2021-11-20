#include "SwapChain.h"
namespace LavaCake {
	namespace Framework {

		SwapChain* SwapChain::m_swapChain;

		void SwapChain::init(
				VkImageUsageFlags													swapchain_image_usage
			) {
			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkSurfaceKHR surface = d->getSurface();
			VkCommandPool pool = d->getCommandPool();

      m_swapchainImageUsage = swapchain_image_usage;
      
			VkResult result = vkDeviceWaitIdle(logical);
			if (result != VK_SUCCESS) {
				ErrorCheck::setError((char*)"Waiting on a device failed.");
			}

			m_images.clear();

			if (!m_handle) {
				InitVkDestroyer(logical, m_handle);
			}
			VkDestroyer(VkSwapchainKHR) old_swapchain = std::move(m_handle);
			InitVkDestroyer(logical, m_handle);
      
      
			/*if (!LavaCake::Core::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(physical, surface, logical, swapchain_image_usage, m_size, m_format, *old_swapchain, *m_handle, m_images)) {
				ErrorCheck::setError((char*)"Can't create swapchain");
			}
			if (!m_handle) {
				ErrorCheck::setError((char*)"Swapchain not created");
			}*/

      
      VkPresentModeKHR desired_present_mode;
      if (!LavaCake::Core::SelectDesiredPresentationMode(physical, surface, VK_PRESENT_MODE_MAILBOX_KHR, desired_present_mode)) {
        ErrorCheck::setError((char*)"Could not select presenter mode");
      }
      
      VkSurfaceCapabilitiesKHR surface_capabilities;
      if (!LavaCake::Core::GetCapabilitiesOfPresentationSurface(physical, surface, surface_capabilities)) {
        ErrorCheck::setError((char*)"Could not get surface capabilities");
      }
      
      uint32_t number_of_images;
      if (!LavaCake::Core::SelectNumberOfSwapchainImages(surface_capabilities, number_of_images)) {
        ErrorCheck::setError((char*)"Could not select number of swapchain image");
      }
      
      if (!LavaCake::Core::ChooseSizeOfSwapchainImages(surface_capabilities, m_size)) {
        ErrorCheck::setError((char*)"Could not set size of swapchain images");
      }
      if ((0 == m_size.width) ||
          (0 == m_size.height)) {
        ErrorCheck::setError((char*)"Wrong swapchain image size");
      }
      
      VkImageUsageFlags image_usage;
      if (!LavaCake::Core::SelectDesiredUsageScenariosOfSwapchainImages(surface_capabilities, swapchain_image_usage, image_usage)) {
        ErrorCheck::setError((char*)"Could not select swapchain image image usage");
      }
      
      VkSurfaceTransformFlagBitsKHR surface_transform;
      LavaCake::Core::SelectTransformationOfSwapchainImages(surface_capabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surface_transform);
      
      VkColorSpaceKHR image_color_space;
      if (!LavaCake::Core::SelectFormatOfSwapchainImages(physical, surface, { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, m_format, image_color_space)) {
        ErrorCheck::setError((char*)"Could not select swapchain image image usage");
      }
      
      if (!LavaCake::Core::CreateSwapchain(logical, surface, number_of_images, { m_format, image_color_space }, m_size, image_usage, surface_transform, desired_present_mode, *old_swapchain, *m_handle)) {
        ErrorCheck::setError((char*)"Could not create the swapchain");
      }
      
      std::vector<VkImage> swapchainImages;
      if (!LavaCake::Core::GetHandlesOfSwapchainImages(logical, *m_handle, swapchainImages)) {
        return false;
      }
      
			for (uint32_t i = 0; i < m_images.size(); ++i) {
				m_swapchainImages.push_back(new SwapChainImage(m_size, m_format, swapchainImages[i]));
			}
		}
  
    void SwapChain::resize(){
      init(m_swapchainImageUsage);
    }
    
	}
}
