#include "SwapChain.h"
namespace LavaCake {
	namespace Framework {

		SwapChain* SwapChain::m_swapChain;

		void SwapChain::init(
				VkImageUsageFlags													swapchain_image_usage,
				bool																			use_depth,
				VkImageUsageFlags													depth_attachment_usage
			) {
			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkSurfaceKHR surface = d->getSurface();
			VkCommandPool pool = d->getCommandPool();

			
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
			if (!LavaCake::Core::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(physical, surface, logical, swapchain_image_usage, m_size, m_format, *old_swapchain, *m_handle, m_images)) {
				ErrorCheck::setError((char*)"Can't create swapchain");
			}
			if (!m_handle) {
				ErrorCheck::setError((char*)"Swapchain not created");
			}

			for (uint32_t i = 0; i < m_images.size(); ++i) {
				m_swapchainImages.push_back(new SwapChainImage(m_size, m_format, m_images[i]));
			}
		}
	}
}
