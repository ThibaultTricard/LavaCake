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
			VkFormat																	m_format = VK_FORMAT_UNDEFINED;
			VkExtent2D																m_size = {uint32_t(0), uint32_t(0)};
			std::vector<VkImage>											m_images;
			std::vector<VkDestroyer(VkImageView)>			m_imageViews;
			std::vector<VkImageView>									m_imageViewsRaw;
			std::vector<VkDestroyer(VkImage)>         m_depthImages;
			std::vector<VkDestroyer(VkDeviceMemory)>  m_depthImagesMemory;
			const VkFormat														m_depthFormat = VK_FORMAT_D16_UNORM;
		};

		class SwapChainImage {
		public: 

			SwapChainImage(VkPhysicalDevice physical, VkDevice logical, VkExtent2D size, VkFormat imageFormat, VkFormat depthFormat) {

				InitVkDestroyer(logical, m_image);
				InitVkDestroyer(logical, m_imageView);
				if (!Image::CreateImageView(logical, *m_image, VK_IMAGE_VIEW_TYPE_2D, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, *m_imageView)) {
					ErrorCheck::setError("Can't create image view");
				}

				InitVkDestroyer(logical, m_depth);
				InitVkDestroyer(logical, m_depthImageMemory);
				InitVkDestroyer(logical, m_depthView);

				if (!Image::Create2DImageAndView(physical, logical, depthFormat, size, 1, 1, VK_SAMPLE_COUNT_1_BIT,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, *m_depth, *m_depthImageMemory,
					*m_depthView)) {
					ErrorCheck::setError("Can't create a depth attachment");
				}
			}

		private:
			VkDestroyer(VkImage)											m_image;
			VkDestroyer(VkImageView)									m_imageView;
			VkDestroyer(VkImage)						          m_depth;
			VkDestroyer(VkImageView)									m_depthView;
			VkDestroyer(VkDeviceMemory)							  m_depthImageMemory;

			VkDestroyer(VkFramebuffer)								m_frameBuffer;
		};
	}

}

