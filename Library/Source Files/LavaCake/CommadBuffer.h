#pragma once

#include "AllHeaders.h"
#include "Device.h"
#include "ErrorCheck.h"


namespace LavaCake {
  namespace Framework {
    class CommandBuffer {
    public:
      CommandBuffer() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkCommandPool pool = d->getCommandPool();
        if (!Command::AllocateCommandBuffers(logical, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, { m_commandBuffer })) {
          ErrorCheck::setError("Failed to allocate commande buffer");
        }

        VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,          // VkStructureType        sType
        nullptr,                                      // const void           * pNext
        VK_FENCE_CREATE_SIGNALED_BIT,                 // VkFenceCreateFlags     flags
        };

        VkResult result = vkCreateFence(logical, &fence_create_info, nullptr, &*m_finishedFence);
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
        VkResult result = vkWaitForFences(logical, static_cast<uint32_t>(1), { &*m_finishedFence }, true, waitingTime);
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Waiting on fence failed." << std::endl;
        }
      }

      void resetFence() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkResult result = vkResetFences(logical, static_cast<uint32_t>(1), { &*m_finishedFence });
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Error occurred when tried to reset fences." << std::endl;
        }
      }

    private:
      VkCommandBuffer                           m_commandBuffer;
      std::vector<VkDestroyer(VkSemaphore)>     m_semaphores;
      VkDestroyer(VkFence)                      m_finishedFence;
    };
  }
}