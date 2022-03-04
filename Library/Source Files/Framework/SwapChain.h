#pragma once

#include "AllHeaders.h"
#include "VulkanFunctions.h"	
#include "Tools.h"
#include "ErrorCheck.h"
#include "Device.h"


 

namespace LavaCake {
	namespace Framework {


		class SwapChainImage {
		public:
			
			SwapChainImage(){}

      SwapChainImage(VkExtent2D /*size*/, VkFormat imageFormat, VkImage image) {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				m_image = image;

				VkImageViewCreateInfo image_view_create_info = {
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType            sType
				nullptr,                                    // const void               * pNext
				0,                                          // VkImageViewCreateFlags     flags
				m_image,                                    // VkImage                    image
				VK_IMAGE_VIEW_TYPE_2D,                      // VkImageViewType            viewType
				imageFormat,                                // VkFormat                   format
				{                                           // VkComponentMapping         components
					VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         r
					VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         g
					VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         b
					VK_COMPONENT_SWIZZLE_IDENTITY               // VkComponentSwizzle         a
				},
				{                                           // VkImageSubresourceRange    subresourceRange
					VK_IMAGE_ASPECT_COLOR_BIT,                  // VkImageAspectFlags         aspectMask
					0,                                          // uint32_t                   baseMipLevel
					VK_REMAINING_MIP_LEVELS,                    // uint32_t                   levelCount
					0,                                          // uint32_t                   baseArrayLayer
					VK_REMAINING_ARRAY_LAYERS                   // uint32_t                   layerCount
				}
				};

				VkResult result = vkCreateImageView(logical, &image_view_create_info, nullptr, &m_imageView);
				if (VK_SUCCESS != result) {
					ErrorCheck::setError("Could not create an image view");
				}

				VkSemaphoreCreateInfo semaphore_create_info = {
					VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType            sType
					nullptr,                                    // const void               * pNext
					0                                           // VkSemaphoreCreateFlags     flags
				};

				result = vkCreateSemaphore(logical, &semaphore_create_info, nullptr, &m_aquiredSemaphore);
				if (VK_SUCCESS != result) {
					ErrorCheck::setError("Could not create a semaphore.");
				}
			}

			VkSemaphore getSemaphore() {
				return m_aquiredSemaphore;
			}

			VkImageView getView() {
				return m_imageView;
			}

			VkImage getImage() {
				return m_image;
			}

			uint32_t getIndex() {
				return m_index;
			}

			~SwapChainImage() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				if (VK_NULL_HANDLE != m_imageView) {
					vkDestroyImageView(logical, m_imageView, nullptr);
					m_imageView = VK_NULL_HANDLE;
				}

				if (m_aquiredSemaphore != VK_NULL_HANDLE) {
					vkDestroySemaphore(logical, m_aquiredSemaphore, nullptr);
				}
			}

		private:
			uint32_t																	m_index =0;
			VkImage																		m_image = VK_NULL_HANDLE;
			VkImageView									              m_imageView = VK_NULL_HANDLE;
			VkSemaphore									              m_aquiredSemaphore = VK_NULL_HANDLE;
			friend class SwapChain;
		};


		class SwapChain {
			static SwapChain* m_swapChain;
			SwapChain() {
				
			};

			~SwapChain() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				for (uint32_t i = 0; i < m_swapchainImages.size(); i++) {
					delete m_swapchainImages[i];
				}
				m_swapchainImages.clear();
				vkDestroySwapchainKHR(logical, m_handle, nullptr);
				m_handle = VK_NULL_HANDLE;

			};

		public :
			
			static SwapChain* getSwapChain() {
				if (!m_swapChain) {
					m_swapChain = new SwapChain();
				}
				return m_swapChain;
			}

			static void destroy() {
				delete m_swapChain;

				
			}

			VkSwapchainKHR& getHandle() {
				return m_handle;
			}

			void init(
				VkImageUsageFlags													swapchain_image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			);

      void resize();
      
			VkFormat imageFormat() {
				return m_format;
			}

			VkFormat depthFormat() {
				return m_depthFormat;
			}
			
			VkExtent2D size() {
				return m_size;
			}

			SwapChainImage& acquireImage() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				VkSemaphoreCreateInfo semaphore_create_info = {
					VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType            sType
					nullptr,                                    // const void               * pNext
					0                                           // VkSemaphoreCreateFlags     flags
				};
				uint32_t index;
				VkSemaphore semaphore;
				VkResult result = vkCreateSemaphore(logical, &semaphore_create_info, nullptr, &semaphore);
				if (VK_SUCCESS != result) {
					ErrorCheck::setError("Could not create a semaphore.");
				}

				result = vkAcquireNextImageKHR(logical, m_handle, 2000000000, semaphore, VK_NULL_HANDLE, &index);
				if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
					ErrorCheck::setError("Could not create a semaphore.");
				}

				if (m_swapchainImages[index]->m_aquiredSemaphore != VK_NULL_HANDLE) {
					vkDestroySemaphore(logical, m_swapchainImages[index]->m_aquiredSemaphore, nullptr);
				}
				

				m_swapchainImages[index]->m_aquiredSemaphore = std::move(semaphore);
				m_swapchainImages[index]->m_index = index;
				return *m_swapchainImages[index];
			}
      
      void presentImage(PresentationQueue& queue, SwapChainImage& image, std::vector<VkSemaphore> semaphores){
        /*Core::PresentInfo present_info = {
          m_handle,                                    // VkSwapchainKHR         Swapchain
          image.getIndex()                              // uint32_t               ImageIndex
        };*/
        
        VkResult result;
        std::vector<VkSwapchainKHR> swapchains;
        std::vector<uint32_t> image_indices;
        
        
        
        
        swapchains.emplace_back(m_handle);
        image_indices.emplace_back(image.getIndex());
        
        
        VkPresentInfoKHR present_info = {
          VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                   // VkStructureType          sType
          nullptr,                                              // const void*              pNext
          static_cast<uint32_t>(semaphores.size()),             // uint32_t                 waitSemaphoreCount
          semaphores.data(),                                    // const VkSemaphore      * pWaitSemaphores
          static_cast<uint32_t>(swapchains.size()),             // uint32_t                 swapchainCount
          swapchains.data(),                                    // const VkSwapchainKHR   * pSwapchains
          image_indices.data(),                                 // const uint32_t         * pImageIndices
          nullptr                                               // VkResult*                pResults
        };
        
        result = vkQueuePresentKHR(queue.getHandle(), &present_info);
        
        if(result != VK_SUCCESS){
          ErrorCheck::setError("Failed to present the image");
        }
      }
      
		private :

			VkSwapchainKHR														m_handle = VK_NULL_HANDLE;
			VkFormat																	m_format = VK_FORMAT_UNDEFINED;
			const VkFormat														m_depthFormat = VK_FORMAT_D16_UNORM;
			VkExtent2D																m_size = {uint32_t(0), uint32_t(0)};

      VkImageUsageFlags                         m_swapchainImageUsage;
      
			std::vector<SwapChainImage*>							m_swapchainImages;

		};


		

	}
}

