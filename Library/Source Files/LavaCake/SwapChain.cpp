#include "SwapChain.h"
namespace LavaCake {
	namespace Framework {

		void SwapChain::init(
				uint32_t																	framesCount,
				VkDevice&																	logicalDevice,
				VkPhysicalDevice&													physicalDevice,
				VkSurfaceKHR&															presentationSurface,
				std::vector<Buffer::FrameResources>&      framesResources,
				VkImageUsageFlags													swapchain_image_usage,
				bool																			use_depth,
				VkImageUsageFlags													depth_attachment_usage
			) {
			Command::WaitForAllSubmittedCommandsToBeFinished(logicalDevice);

			m_framesCount = framesCount;
			m_imageViewsRaw.clear();
			m_imageViews.clear();
			m_images.clear();

			if (!m_handle) {
				InitVkDestroyer(logicalDevice, m_handle);
			}
			VkDestroyer(VkSwapchainKHR) old_swapchain = std::move(m_handle);
			InitVkDestroyer(logicalDevice, m_handle);
			if (!Swapchain::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(physicalDevice, presentationSurface, logicalDevice, swapchain_image_usage, m_size, m_format, *old_swapchain, *m_handle, m_images)) {
				ErrorCheck::setError("Can't create swapchain");
			}
			if (!m_handle) {
				ErrorCheck::setError("Swapchain not created");
			}

			for (size_t i = 0; i < m_images.size(); ++i) {
				m_imageViews.emplace_back(VkDestroyer(VkImageView)());
				InitVkDestroyer(logicalDevice, m_imageViews.back());
				if (!Image::CreateImageView(logicalDevice, m_images[i], VK_IMAGE_VIEW_TYPE_2D, m_format, VK_IMAGE_ASPECT_COLOR_BIT, *m_imageViews.back())) {
					ErrorCheck::setError("Can't create image view");
				}
				m_imageViewsRaw.push_back(*m_imageViews.back());
			}

			m_depthImages.clear();
			m_depthImagesMemory.clear();

			if (use_depth) {
				for (uint32_t i = 0; i < m_framesCount; ++i) {
					m_depthImages.emplace_back(VkDestroyer(VkImage)());
					InitVkDestroyer(logicalDevice, m_depthImages.back());
					m_depthImagesMemory.emplace_back(VkDestroyer(VkDeviceMemory)());
					InitVkDestroyer(logicalDevice, m_depthImagesMemory.back());
					InitVkDestroyer(logicalDevice, (framesResources)[i].depthAttachment);

					if (!Image::Create2DImageAndView(physicalDevice, logicalDevice, m_depthFormat, m_size, 1, 1, VK_SAMPLE_COUNT_1_BIT,
						VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, *m_depthImages.back(), *m_depthImagesMemory.back(),
						*(framesResources)[i].depthAttachment)) {
						ErrorCheck::setError("Can't create frames");
					}
				}
			}
		}
	}

}