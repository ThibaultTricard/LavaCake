#pragma once

#include "AllHeaders.h"
#include "Device.h"
#include "ErrorCheck.h"
#include <cassert>
#include <utility>

namespace LavaCake {
  namespace Framework {
    /**
    \brief help manage Vulkan semaphores.
    */
    class Semaphore {
    
    public :

      /**
      \brief Default Constructor, create a semaphores
      */
      Semaphore() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkSemaphoreCreateInfo semaphore_create_info = {
          VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType            sType
          nullptr,                                    // const void               * pNext
          0                                           // VkSemaphoreCreateFlags     flags
        };

        VkResult result = vkCreateSemaphore(logical, &semaphore_create_info, nullptr, &m_semaphore);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Failed to create a semaphore for the commande buffer");
        }
      }

      /**
      \brief Create a semaphores from a VKSemaphore
      \param semaphore the VkSemaphore
      */
      Semaphore(VkSemaphore semaphore) {
        m_semaphore = semaphore;
      }

      Semaphore(const Semaphore& semaphore) = delete;
      Semaphore& operator=(const Semaphore&) = delete;

      Semaphore(Semaphore&& s) noexcept
        : m_semaphore(std::exchange(s.m_semaphore, VK_NULL_HANDLE))
      {}

      Semaphore& operator=(Semaphore&& s) noexcept
      {
        if (this != &s)
        {
          m_semaphore = std::exchange(s.m_semaphore, VK_NULL_HANDLE);
        }
        return *this;
      }


      /**
      \brief Returns the handle to the semaphore
      \return semaphore the VkSemaphore
      */
      VkSemaphore getHandle() const{
        return m_semaphore;
      }


      ~Semaphore(){
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        if (m_semaphore != VK_NULL_HANDLE) {
          vkDestroySemaphore(logical, m_semaphore, nullptr);
        }
      }


    private :
      VkSemaphore m_semaphore = VK_NULL_HANDLE;
    };



    struct waitSemaphoreInfo {
      std::shared_ptr<Semaphore>           semaphore;
      VkPipelineStageFlags  waitingStage;
    };

    /**
    Class CommandBuffer :
    \brief Helps manage VkCommandBuffer and their synchornisation
    */
    class CommandBuffer {
    public:

      /**
      \brief Constructor the CommandBuffer class.
      */
      CommandBuffer() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkCommandPool pool = d->getCommandPool();

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // VkStructureType          sType
        nullptr,                                          // const void             * pNext
        pool,                                     // VkCommandPool            commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                                            // VkCommandBufferLevel     level
        1                                             // uint32_t                 commandBufferCount
        };


        VkResult result = vkAllocateCommandBuffers(logical, &command_buffer_allocate_info, &m_commandBuffer);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Failed to allocate commande buffer.");
        }

        VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,          // VkStructureType        sType
        nullptr,                                      // const void           * pNext
        VK_FENCE_CREATE_SIGNALED_BIT,                 // VkFenceCreateFlags     flags
        };

        result = vkCreateFence(logical, &fence_create_info, nullptr, &m_fence);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Could not create a fence.");
        }
      };

      /**
      \brief Wait for the CommandBuffer to be executed if it was submitted
      \param waitingTime (optional) the maximum waiting time allowed to this function in nanoseconds
      \param force (optional) if set to true, will wait even if it was not submited
      */
      void wait(uint32_t waitingTime = UINT32_MAX, bool force = false) {
        if (m_submitted || force) {
          Device* d = Device::getDevice();
          VkDevice logical = d->getLogicalDevice();

          VkResult result = vkWaitForFences(logical, static_cast<uint32_t>(1), &m_fence, true, waitingTime);
          if (VK_SUCCESS != result) {
            ErrorCheck::setError("Waiting on fence failed");
          }
        }
        m_submitted = false;
      }

      /**
      \brief Reset the fence associated with this buffer.
      Must be called before re-submiting the command buffer
      */
      void resetFence() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkResult result = vkResetFences(logical, static_cast<uint32_t>(1), &m_fence);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Error occurred when tried to reset fences");
        }
      }

      /**
      \brief Put the command buffer in a recording state
      */
      void beginRecord() {
        VkCommandBufferBeginInfo command_buffer_begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                        sType
        nullptr,                                        // const void                           * pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,    // VkCommandBufferUsageFlags              flags
        nullptr                                         // const VkCommandBufferInheritanceInfo * pInheritanceInfo
        };

        VkResult result = vkBeginCommandBuffer(m_commandBuffer, &command_buffer_begin_info);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Could not begin command buffer recording operation.");
        }
      }

      /**
      \brief Put the command buffer out of recording state
      */
      void endRecord() {
        VkResult result = vkEndCommandBuffer(m_commandBuffer);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Error occurred during command buffer recording.");
        }
      }

      /**
      \brief Return the handle of command buffer
      \return a handle to the VkCommandBuffer
      */
      const VkCommandBuffer& getHandle() const {
        return m_commandBuffer;
      }

      /**
      \brief Returns the fence of the command buffer
      \return a handle to a VkFence
      */
      const VkFence& getFence() const {
        return m_fence;
      }

      /**
      \brief Submit the command buffer to a queue
      \param queue : a pointer to the queue that will be used to submit this command buffer
      \param waitSemaphoreInfo : description of the semaphores to to wait on before executing it
      \param signalSemaphores : the list of that will be raised by the execution of this command buffer
      */
      void submit(const Queue& queue, const std::vector<waitSemaphoreInfo>& waitSemaphoreInfo, const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores) {
        std::vector<VkSemaphore>          wait_semaphore_handles;
        std::vector<VkPipelineStageFlags> wait_semaphore_stages;
        for (auto& wait_semaphore_info : waitSemaphoreInfo) {
          wait_semaphore_handles.emplace_back(wait_semaphore_info.semaphore->getHandle());
          wait_semaphore_stages.emplace_back(wait_semaphore_info.waitingStage);
        }

        std::vector<VkSemaphore>          signaled_semaphore_handles;
        for (auto& signaled : signalSemaphores) {
          signaled_semaphore_handles.emplace_back(signaled->getHandle());
        }

        std::vector<VkCommandBuffer> command_buffers = { getHandle() };

        VkSubmitInfo submit_info = {
          VK_STRUCTURE_TYPE_SUBMIT_INFO,                                 // VkStructureType                sType
          nullptr,                                                       // const void                   * pNext
          static_cast<uint32_t>(waitSemaphoreInfo.size()),			         // uint32_t                       waitSemaphoreCount
          wait_semaphore_handles.data(),                                 // const VkSemaphore            * pWaitSemaphores
          wait_semaphore_stages.data(),                                  // const VkPipelineStageFlags   * pWaitDstStageMask
          static_cast<uint32_t>(1),															         // uint32_t                       commandBufferCount
          command_buffers.data(),                                        // const VkCommandBuffer        * pCommandBuffers
          static_cast<uint32_t>(signaled_semaphore_handles.size()),      // uint32_t                       signalSemaphoreCount
          signaled_semaphore_handles.data()                              // const VkSemaphore            * pSignalSemaphores
        };

        VkResult result = vkQueueSubmit(queue.getHandle(), 1, &submit_info, getFence());
        if (VK_SUCCESS != result) {
          ErrorCheck::setError("Error occurred during command buffer submission.");
          return;
        }
        m_submitted = true;
      }


      /**
      \brief Check if the fence of the command buffer has been raised
      \return bool : a boolean indicating the fence has been raised
      */
      bool ready() const {
        auto device = Device::getDevice()->getLogicalDevice();
        VkResult res = vkGetFenceStatus(device, m_fence);
        if (res == VK_SUCCESS) {
          return true;
        }
        return false;
      }

      ~CommandBuffer() {

        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        if (m_fence != VK_NULL_HANDLE) {
          vkDestroyFence(logical, m_fence, nullptr);
        }
        if (m_commandBuffer != VK_NULL_HANDLE) {
          vkFreeCommandBuffers(logical, d->getCommandPool(), 1, &m_commandBuffer);
        }
      };

      

    private:
      VkCommandBuffer                           m_commandBuffer = VK_NULL_HANDLE;
      VkFence                                   m_fence = VK_NULL_HANDLE;

      bool                                      m_submitted = false;
    };
  }
}
