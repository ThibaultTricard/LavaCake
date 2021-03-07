#pragma once

#include "AllHeaders.h"
#include "Device.h"
#include "ErrorCheck.h"
#include <cassert>

namespace LavaCake {
  namespace Framework {
    class CommandBuffer {
    public:
      CommandBuffer() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkCommandPool pool = d->getCommandPool();
        std::vector<VkCommandBuffer> buffers = { m_commandBuffer };

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // VkStructureType          sType
        nullptr,                                          // const void             * pNext
        pool,                                     // VkCommandPool            commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                                            // VkCommandBufferLevel     level
        1                                             // uint32_t                 commandBufferCount
        };


        VkResult result = vkAllocateCommandBuffers(logical, &command_buffer_allocate_info, buffers.data());
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Failed to allocate commande buffer");
        }

        m_commandBuffer = buffers[0];

        VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,          // VkStructureType        sType
        nullptr,                                      // const void           * pNext
        VK_FENCE_CREATE_SIGNALED_BIT,                 // VkFenceCreateFlags     flags
        };

        result = vkCreateFence(logical, &fence_create_info, nullptr, &*m_fence);
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Could not create a fence." << std::endl;
        }
      };

      void addSemaphore() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        m_semaphores.emplace_back(VkDestroyer(VkSemaphore)());
        VkSemaphoreCreateInfo semaphore_create_info = {
          VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType            sType
          nullptr,                                    // const void               * pNext
          0                                           // VkSemaphoreCreateFlags     flags
        };

        VkResult result = vkCreateSemaphore(logical, &semaphore_create_info, nullptr, &*m_semaphores.back());
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Could not create a semaphore." << std::endl;
        }
      }

      void wait(uint32_t waitingTime) {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkResult result = vkWaitForFences(logical, static_cast<uint32_t>(1), { &*m_fence }, true, waitingTime);
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Waiting on fence failed." << std::endl;
        }
      }

      void resetFence() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkResult result = vkResetFences(logical, static_cast<uint32_t>(1), { &*m_fence });
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Error occurred when tried to reset fences." << std::endl;
        }
      }

      void beginRecord() {
        VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                        sType
        nullptr,                                        // const void                           * pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,    // VkCommandBufferUsageFlags              flags
        nullptr                                         // const VkCommandBufferInheritanceInfo * pInheritanceInfo
        };

        VkResult result = vkBeginCommandBuffer(m_commandBuffer, &command_buffer_begin_info);
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Could not begin command buffer recording operation." << std::endl;
        }
      }

      void endRecord() {
        VkResult result = vkEndCommandBuffer(m_commandBuffer);
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Error occurred during command buffer recording." << std::endl;
        }
      }

      VkCommandBuffer& getHandle() {
        return m_commandBuffer;
      }

      VkSemaphore& getSemaphore(int i) {
        return *m_semaphores[i];
      }

      VkFence& getFence() {
        return *m_fence;
      }

      void submit(Queue* queue, std::vector<Core::WaitSemaphoreInfo> waitSemaphoreInfo, std::vector<VkSemaphore>  signal_semaphores) {
        std::vector<VkSemaphore>          wait_semaphore_handles;
        std::vector<VkPipelineStageFlags> wait_semaphore_stages;
        for (auto& wait_semaphore_info : waitSemaphoreInfo) {
          wait_semaphore_handles.emplace_back(wait_semaphore_info.Semaphore);
          wait_semaphore_stages.emplace_back(wait_semaphore_info.WaitingStage);
        }

        std::vector<VkCommandBuffer> command_buffers = { getHandle() };

        VkSubmitInfo submit_info = {
          VK_STRUCTURE_TYPE_SUBMIT_INFO,                        // VkStructureType                sType
          nullptr,                                              // const void                   * pNext
          static_cast<uint32_t>(waitSemaphoreInfo.size()),			// uint32_t                       waitSemaphoreCount
          wait_semaphore_handles.data(),                        // const VkSemaphore            * pWaitSemaphores
          wait_semaphore_stages.data(),                         // const VkPipelineStageFlags   * pWaitDstStageMask
          static_cast<uint32_t>(1),															// uint32_t                       commandBufferCount
          command_buffers.data(),                               // const VkCommandBuffer        * pCommandBuffers
          static_cast<uint32_t>(signal_semaphores.size()),      // uint32_t                       signalSemaphoreCount
          signal_semaphores.data()                              // const VkSemaphore            * pSignalSemaphores
        };

        VkResult result = vkQueueSubmit(queue->getHandle(), 1, &submit_info, getFence());
        if (VK_SUCCESS != result) {
          std::cout << "Error occurred during command buffer submission." << std::endl;
        }
      }


      ~CommandBuffer() {

        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        for (size_t t = 0; t < m_semaphores.size(); t++) {
          if (*m_semaphores[t] != VK_NULL_HANDLE) {
            vkDestroySemaphore(logical, *m_semaphores[t], nullptr);
          }
        }
        if (*m_fence != VK_NULL_HANDLE) {
          vkDestroyFence(logical, *m_fence, nullptr);
        }
        if (m_commandBuffer != VK_NULL_HANDLE) {
          std::vector<VkCommandBuffer> buffers = { m_commandBuffer };
          vkFreeCommandBuffers(logical, d->getCommandPool(), 1, &buffers[0]);
        }
      };

    private:
      VkCommandBuffer                           m_commandBuffer;
      std::vector<VkDestroyer(VkSemaphore)>     m_semaphores;
      VkDestroyer(VkFence)                      m_fence;
    };
  }
}