#pragma once

#include "AllHeaders.h"
#include "VulkanFunctions.h"	
#include "VulkanDestroyer.h"
#include "Tools.h"
#include "ErrorCheck.h"

#ifdef _WIN32
#include <Windows.h>
#endif
 

namespace LavaCake {
	namespace Framework {

		class SwapChain {

		public :
			SwapChain() {};

			VkSwapchainKHR& getHandle() {
				return *m_handle;
			}

			std::vector<VkImageView>& getImageView() {
				return m_imageViewsRaw;
			}

			std::vector<VkImage>& getImages() {
				return m_images;
			}

			void init(
				uint32_t																	framesCount,
				VkDevice&																	logicalDevice,
				VkPhysicalDevice&													physicalDevice,
				VkSurfaceKHR&															presentationSurface,
				std::vector<Buffer::FrameResources>&      framesResources,
				VkImageUsageFlags													swapchain_image_usage,
				bool																			use_depth,
				VkImageUsageFlags													depth_attachment_usage
			);

			VkFormat imageFormat() {
				return m_format;
			}

			VkFormat depthFormat() {
				return m_depthFormat;
			}
			
			
			VkExtent2D size() {
				return m_size;
			}
		private :
			uint32_t																	m_framesCount = 0;;
			VkDestroyer(VkSwapchainKHR)								m_handle;
			VkFormat																	m_format;
			VkExtent2D																m_size;
			std::vector<VkImage>											m_images;
			std::vector<VkDestroyer(VkImageView)>			m_imageViews;
			std::vector<VkImageView>									m_imageViewsRaw;
			std::vector<VkDestroyer(VkImage)>         m_depthImages;
			std::vector<VkDestroyer(VkDeviceMemory)>  m_depthImagesMemory;
			const VkFormat														m_depthFormat = VK_FORMAT_D16_UNORM;
		};
	}

}

