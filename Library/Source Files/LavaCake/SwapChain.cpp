#include "SwapChain.h"
namespace LavaCake {
	namespace Framework {

		SwapChain* SwapChain::m_swapChain;

		void SwapChain::init(
				uint32_t																	framesCount,
				VkImageUsageFlags													swapchain_image_usage,
				bool																			use_depth,
				VkImageUsageFlags													depth_attachment_usage
			) {
			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkSurfaceKHR surface = d->getSurface();
			VkCommandPool pool = d->getCommandPool();


			

			for (uint32_t i = 0; i < framesCount; ++i) {
				std::vector<VkCommandBuffer> command_buffer;
				VkDestroyer(VkSemaphore) image_acquired_semaphore;
				InitVkDestroyer(logical, image_acquired_semaphore);
				VkDestroyer(VkSemaphore) ready_to_present_semaphore;
				InitVkDestroyer(logical, ready_to_present_semaphore);
				VkDestroyer(VkFence) drawing_finished_fence;
				InitVkDestroyer(logical, drawing_finished_fence);
				VkDestroyer(VkImageView) depth_attachment;
				InitVkDestroyer(logical, depth_attachment);

				if (!Command::AllocateCommandBuffers(logical, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, command_buffer)) {
					ErrorCheck::setError("Failed to allocate commande buffer");
				}
				if (!Semaphore::CreateSemaphore(logical, *image_acquired_semaphore)) {
					ErrorCheck::setError("Failed to create commande semaphore");
				}
				if (!Semaphore::CreateSemaphore(logical, *ready_to_present_semaphore)) {
					ErrorCheck::setError("Failed to create commande semaphore");
				}
				if (!Fence::CreateFence(logical, true, *drawing_finished_fence)) {
					ErrorCheck::setError("Failed to create commande fence");
				}

				m_framesResources.emplace_back(
					command_buffer[0],
					std::move(image_acquired_semaphore),
					std::move(ready_to_present_semaphore),
					std::move(drawing_finished_fence),
					std::move(depth_attachment),
					VkDestroyer(VkFramebuffer)()
				);
			}

			Command::WaitForAllSubmittedCommandsToBeFinished(logical);

			m_framesCount = framesCount;
			m_imageViewsRaw.clear();
			m_imageViews.clear();
			m_images.clear();

			if (!m_handle) {
				InitVkDestroyer(logical, m_handle);
			}
			VkDestroyer(VkSwapchainKHR) old_swapchain = std::move(m_handle);
			InitVkDestroyer(logical, m_handle);
			if (!Swapchain::CreateSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(physical, surface, logical, swapchain_image_usage, m_size, m_format, *old_swapchain, *m_handle, m_images)) {
				ErrorCheck::setError("Can't create swapchain");
			}
			if (!m_handle) {
				ErrorCheck::setError("Swapchain not created");
			}

			for (uint32_t i = 0; i < m_images.size(); ++i) {
				m_swapchainImages.push_back(SwapChainImage(m_size, m_format, m_depthFormat, m_images[i]));
			}

			for (size_t i = 0; i < m_images.size(); ++i) {
				m_imageViews.emplace_back(VkDestroyer(VkImageView)());
				InitVkDestroyer(logical, m_imageViews.back());
				if (!Image::CreateImageView(logical, m_images[i], VK_IMAGE_VIEW_TYPE_2D, m_format, VK_IMAGE_ASPECT_COLOR_BIT, *m_imageViews.back())) {
					ErrorCheck::setError("Can't create image view");
				}
				m_imageViewsRaw.push_back(*m_imageViews.back());
			}

			m_depthImages.clear();
			m_depthImagesMemory.clear();

			if (use_depth) {
				for (uint32_t i = 0; i < m_framesCount; ++i) {
					m_depthImages.emplace_back(VkDestroyer(VkImage)());
					InitVkDestroyer(logical, m_depthImages.back());
					m_depthImagesMemory.emplace_back(VkDestroyer(VkDeviceMemory)());
					InitVkDestroyer(logical, m_depthImagesMemory.back());
					InitVkDestroyer(logical, (m_framesResources)[i].depthAttachment);

					if (!Image::Create2DImageAndView(physical, logical, m_depthFormat, m_size, 1, 1, VK_SAMPLE_COUNT_1_BIT,
						VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, *m_depthImages.back(), *m_depthImagesMemory.back(),
						*(m_framesResources)[i].depthAttachment)) {
						ErrorCheck::setError("Can't create frames");
					}
				}
			}
		}
		
	}
}