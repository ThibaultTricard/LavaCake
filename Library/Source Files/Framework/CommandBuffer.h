#pragma once

#include "AllHeaders.h"
#include "Device.h"
#include "ErrorCheck.h"
#include <cassert>

namespace LavaCake {
  namespace Framework {
  
  /**
   Class CommandBuffer :
   \brief Helps manage VkCommandBuffer and their synchornisation
   */
    class CommandBuffer {
    public:
      
      /**
       Constructor the CommandBuffer class
       \brief Initialise a VkCommandBuffer and a VkFence for it's synchronisation
       */
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
          ErrorCheck::setError((char*)"Failed to allocate commande buffer");
        }

        m_commandBuffer = buffers[0];

        VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,          // VkStructureType        sType
        nullptr,                                      // const void           * pNext
        VK_FENCE_CREATE_SIGNALED_BIT,                 // VkFenceCreateFlags     flags
        };

        result = vkCreateFence(logical, &fence_create_info, nullptr, &m_fence);
        if (VK_SUCCESS != result) {
          //TODO : Raise error using error check
          //std::cout << "Could not create a fence." << std::endl;
        }
      };
      
      /**
       \brief Create a VkSemaphore and add it into an intern list
       */
      void addSemaphore() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        m_semaphores.emplace_back(VkSemaphore());
        VkSemaphoreCreateInfo semaphore_create_info = {
          VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType            sType
          nullptr,                                    // const void               * pNext
          0                                           // VkSemaphoreCreateFlags     flags
        };

        VkResult result = vkCreateSemaphore(logical, &semaphore_create_info, nullptr, &m_semaphores.back());
        if (VK_SUCCESS != result) {
          ErrorCheck::setError((char*)"Failed to create a semaphore for the commande buffer");
        }
      }

      /**
       \brief Wait for the CommandBuffer to be executed if it was submitted
       \param waitingTime (optional) the maximum waiting time allowed to this function in nanoseconds
       \param force (optional) if set to true, will wait even if it was not submited
       */
      void wait(uint32_t waitingTime = UINT32_MAX, bool force = false) {
        if(m_submitted || force){
          Device* d = Device::getDevice();
          VkDevice logical = d->getLogicalDevice();
          
          VkResult result = vkWaitForFences(logical, static_cast<uint32_t>(1),  &m_fence , true, waitingTime);
          if (VK_SUCCESS != result) {
            ErrorCheck::setError((char*)"Waiting on fence failed");
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
        VkResult result = vkResetFences(logical, static_cast<uint32_t>(1),  &m_fence );
        if (VK_SUCCESS != result) {
          ErrorCheck::setError((char*)"Error occurred when tried to reset fences");
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
          ErrorCheck::setError((char*)"Could not begin command buffer recording operation.");
        }
      }
      
      /**
       \brief Put the command buffer out of recording state
       */
      void endRecord() {
        VkResult result = vkEndCommandBuffer(m_commandBuffer);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError((char*)"Error occurred during command buffer recording.");
        }
      }

      /**
       \brief Return the handle of command buffer
       \return a handle to the VkCommandBuffer
       */
      VkCommandBuffer& getHandle() {
        return m_commandBuffer;
      }
      
      /**
       \brief Return the semaphore at a given index
       \param i the index of the wanted semaphore
       \return a handle to a VkSemaphore
       */
      VkSemaphore& getSemaphore(int i) {
        return m_semaphores[i];
      }

      /**
       \brief Return the fence of the command buffer
       \return a handle to a VkFence
       */
      VkFence& getFence() {
        return m_fence;
      }

      /**
       \brief Submit the command buffer to a queue
       \param queue : a pointer to the queue that will be used to submit this command buffer
       \param waitSemaphoreInfo : description of the semaphores to to wait on before executing it
       \param signalSemaphores : the list of that will be raised by the execution of this command buffer
       */
      void submit(Queue* queue, std::vector<Core::WaitSemaphoreInfo> waitSemaphoreInfo, std::vector<VkSemaphore>  signalSemaphores) {
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
          static_cast<uint32_t>(signalSemaphores.size()),      // uint32_t                       signalSemaphoreCount
          signalSemaphores.data()                              // const VkSemaphore            * pSignalSemaphores
        };

        VkResult result = vkQueueSubmit(queue->getHandle(), 1, &submit_info, getFence());
        if (VK_SUCCESS != result) {
          ErrorCheck::setError((char*)"Error occurred during command buffer submission.");
          return;
        }
        m_submitted =true;
      }


      ~CommandBuffer() {

        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        for (size_t t = 0; t < m_semaphores.size(); t++) {
          if (m_semaphores[t] != VK_NULL_HANDLE) {
            vkDestroySemaphore(logical, m_semaphores[t], nullptr);
          }
        }
        if (m_fence != VK_NULL_HANDLE) {
          vkDestroyFence(logical, m_fence, nullptr);
        }
        if (m_commandBuffer != VK_NULL_HANDLE) {
          std::vector<VkCommandBuffer> buffers = { m_commandBuffer };
          vkFreeCommandBuffers(logical, d->getCommandPool(), 1, &buffers[0]);
        }
      };

      bool ready(){
        auto device = Device::getDevice()->getLogicalDevice();
        VkResult res = vkGetFenceStatus(device, m_fence);
        if(res == VK_SUCCESS){
          return true;
        }
        return false;
      }
      
    private:
      VkCommandBuffer                           m_commandBuffer = VK_NULL_HANDLE;
      std::vector<VkSemaphore>                  m_semaphores;
      VkFence                                   m_fence = VK_NULL_HANDLE;
        
      bool                                      m_submitted = false;
    };
  }
}
