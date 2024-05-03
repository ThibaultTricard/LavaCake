#pragma once

#include "AllHeaders.h"
#include "VulkanFunctions.h"	
#include "Tools.h"
#include "ErrorCheck.h"
#include "Device.h"
#include "CommandBuffer.h"




namespace LavaCake {
  namespace Framework {


    class SwapChainImage {
    public:

      SwapChainImage() {}

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

      }

      std::shared_ptr<Semaphore> getSemaphore() const {
        return m_aquiredSemaphore;
      }

      const VkImageView& getView() const {
        return m_imageView;
      }

      const VkImage getImage() const {
        return m_image;
      }

      uint32_t getIndex() const {
        return m_index;
      }

      ~SwapChainImage() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        if (VK_NULL_HANDLE != m_imageView) {
          vkDestroyImageView(logical, m_imageView, nullptr);
          m_imageView = VK_NULL_HANDLE;
        }
      }

    private:
      uint32_t																	m_index = 0;
      VkImage																		m_image = VK_NULL_HANDLE;
      VkImageView									              m_imageView = VK_NULL_HANDLE;
      std::shared_ptr<Semaphore>								m_aquiredSemaphore = nullptr;
      friend class SwapChain;
    };


    class SwapChain {
      inline static SwapChain* m_swapChain = nullptr;
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

    public:

      static SwapChain* getSwapChain() {
        if (!m_swapChain) {
          m_swapChain = new SwapChain();
        }
        return m_swapChain;
      }

      static void destroy() {
        delete m_swapChain;
      }

      const VkSwapchainKHR& getHandle() const {
        return m_handle;
      }

      void init(
        VkImageUsageFlags													swapchain_image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VkPresentModeKHR                          desired_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR
      );

      void resize();

      VkFormat imageFormat() const {
        return m_format;
      }


      VkExtent2D size() const {
        return m_size;
      }

      const SwapChainImage& acquireImage() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkSemaphoreCreateInfo semaphore_create_info = {
          VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType            sType
          nullptr,                                    // const void               * pNext
          0                                           // VkSemaphoreCreateFlags     flags
        };
        uint32_t index;
        

        std::shared_ptr<Semaphore> s = std::make_shared <Semaphore>();


        VkResult result = vkAcquireNextImageKHR(logical, m_handle, 2000000000, s->getHandle(), VK_NULL_HANDLE, &index);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
          ErrorCheck::setError("Could not aquire the swapchain image.");
        }

        m_swapchainImages[index]->m_aquiredSemaphore = s;
        m_swapchainImages[index]->m_index = index;
        return *m_swapchainImages[index];
      }

      void presentImage(const PresentationQueue& queue, const SwapChainImage& image, const std::vector<std::shared_ptr<Semaphore>>& semaphores) const {
        /*Core::PresentInfo present_info = {
          m_handle,                                    // VkSwapchainKHR         Swapchain
          image.getIndex()                              // uint32_t               ImageIndex
        };*/

        VkResult result;
        std::vector<VkSwapchainKHR> swapchains;
        std::vector<uint32_t> image_indices;

        std::vector<VkSemaphore> semaphoreHandles;
        for (auto& s: semaphores) {
          semaphoreHandles.push_back(s->getHandle());
        }

        swapchains.emplace_back(m_handle);
        image_indices.emplace_back(image.getIndex());


        VkPresentInfoKHR present_info = {
          VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                   // VkStructureType          sType
          nullptr,                                              // const void*              pNext
          static_cast<uint32_t>(semaphores.size()),             // uint32_t                 waitSemaphoreCount
          semaphoreHandles.data(),                              // const VkSemaphore      * pWaitSemaphores
          static_cast<uint32_t>(swapchains.size()),             // uint32_t                 swapchainCount
          swapchains.data(),                                    // const VkSwapchainKHR   * pSwapchains
          image_indices.data(),                                 // const uint32_t         * pImageIndices
          nullptr                                               // VkResult*                pResults
        };

        result = vkQueuePresentKHR(queue.getHandle(), &present_info);

        if (result != VK_SUCCESS) {
          ErrorCheck::setError("Failed to present the image");
        }
      }

    private:

      VkSwapchainKHR														m_handle = VK_NULL_HANDLE;
      VkFormat																	m_format = VK_FORMAT_UNDEFINED;
      VkExtent2D																m_size = { uint32_t(0), uint32_t(0) };

      VkImageUsageFlags                         m_swapchainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

      std::vector<SwapChainImage*>							m_swapchainImages;

    };




  }
}

