#pragma once

#include "AllHeaders.h"
#include "VulkanFunctions.h"	
#include "VulkanDestroyer.h"
#include "Tools.h"
#include "ErrorCheck.h"
#include "Device.h"


 

namespace LavaCake {
	namespace Framework {


		class SwapChainImage {
		public:

			SwapChainImage(VkExtent2D size, VkFormat imageFormat, VkFormat depthFormat, VkImage image) {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				VkPhysicalDevice physical = d->getPhysicalDevice();
				m_image = image;
				InitVkDestroyer(logical, m_imageView);
				if (!Image::CreateImageView(logical, m_image, VK_IMAGE_VIEW_TYPE_2D, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, *m_imageView)) {
					ErrorCheck::setError("Can't create image view");
				}
			}

		private:
			VkImage																		m_image;
			VkDestroyer(VkImageView)									m_imageView;
		};


		class SwapChain {
			static SwapChain* m_swapChain;
			SwapChain() {};
		public :
			
			static SwapChain* getSwapChain() {
				if (!m_swapChain) {
					m_swapChain = new SwapChain();
				}
				return m_swapChain;
			}

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
				VkImageUsageFlags													swapchain_image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				bool																			use_depth = true,
				VkImageUsageFlags													depth_attachment_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
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

			std::vector<Buffer::FrameResources>* getFrameRessources() {
				return &m_framesResources;
			}

		private :

			
			VkDestroyer(VkSwapchainKHR)								m_handle;
			VkFormat																	m_format = VK_FORMAT_UNDEFINED;
			const VkFormat														m_depthFormat = VK_FORMAT_D16_UNORM;
			VkExtent2D																m_size = {uint32_t(0), uint32_t(0)};
			uint32_t																	m_framesCount = 0;;

			std::vector<SwapChainImage>								m_swapchainImages;

			std::vector<Buffer::FrameResources>				m_framesResources;
			std::vector<VkImage>											m_images;
			std::vector<VkDestroyer(VkImageView)>			m_imageViews;
			std::vector<VkImageView>									m_imageViewsRaw;
			std::vector<VkDestroyer(VkImage)>         m_depthImages;
			std::vector<VkDestroyer(VkDeviceMemory)>  m_depthImagesMemory;
		};


		

	}
}

