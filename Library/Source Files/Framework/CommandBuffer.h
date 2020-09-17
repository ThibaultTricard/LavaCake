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
        if (!LavaCake::Command::AllocateCommandBuffers(logical, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, buffers)) {
          ErrorCheck::setError("Failed to allocate commande buffer");
        }
        m_commandBuffer = buffers[0];

        VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,          // VkStructureType        sType
        nullptr,                                      // const void           * pNext
        VK_FENCE_CREATE_SIGNALED_BIT,                 // VkFenceCreateFlags     flags
        };

        VkResult result = vkCreateFence(logical, &fence_create_info, nullptr, &*m_fence);
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
        if (!LavaCake::Command::BeginCommandBufferRecordingOperation(m_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
          //TODO : Raise error using error check
        }
      }

      void endRecord() {
        if (!LavaCake::Command::EndCommandBufferRecordingOperation(m_commandBuffer)) {
          assert("ERROR");
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
          LavaCake::Command::FreeCommandBuffers(logical, d->getCommandPool(), buffers);
        }
      };

    private:
      VkCommandBuffer                           m_commandBuffer;
      std::vector<VkDestroyer(VkSemaphore)>     m_semaphores;
      VkDestroyer(VkFence)                      m_fence;
    };
  }
}