#pragma once

#include "CommandBuffer.h"
#include "Queue.h"

namespace LavaCake {
  namespace Framework {
    
    class Buffer {
    public :

			Buffer() {

			}


			template <typename t>
			void allocate(Queue* queue, CommandBuffer& cmdBuff, std::vector<t> rawdata, VkBufferUsageFlagBits usage, VkFormat format = VK_FORMAT_R32_SFLOAT, VkAccessFlagBits accessmod = VK_ACCESS_TRANSFER_WRITE_BIT, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_TRANSFER_BIT) {
				Device* d = Device::getDevice();
				VkPhysicalDevice physical = d->getPhysicalDevice();
				VkDevice logical = d->getLogicalDevice();
				m_dataSize = rawdata.size();


				Core::DestroyBuffer(logical, *m_buffer);
				Core::DestroyBufferView(logical, *m_bufferView);
				Core::FreeMemoryObject(logical, *m_bufferMemory);

				if (!LavaCake::Core::CreateBuffer(logical, sizeof(t) * rawdata.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, *m_buffer)) {
					ErrorCheck::setError("Can't create Buffer");
				}

				if (!LavaCake::Core::AllocateAndBindMemoryObjectToBuffer(physical, logical, *m_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_bufferMemory)) {
					ErrorCheck::setError("Can't allocate Buffer");
				}

				if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT){
					if (!LavaCake::Core::CreateBufferView(logical, *m_buffer, format, 0, VK_WHOLE_SIZE, *m_bufferView)) {
						ErrorCheck::setError("Can't create Buffer view");
					}
				}


				if (!LavaCake::Core::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physical, logical, sizeof(rawdata[0]) * rawdata.size(), &rawdata[0],
					*m_buffer, 0, 0, accessmod, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, stageFlagBit, queue->getHandle(), cmdBuff.getHandle() , {})) {
					ErrorCheck::setError("Can't copy data to buffer");
				}
      }

			VkBuffer& getHandle() {
				return *m_buffer;
			}

			VkBufferView& getBufferView() {
				return *m_bufferView;
			}

			void readBack(Queue* queue, CommandBuffer& cmdBuff, std::vector<float>& data) {
				Device* d = Device::getDevice();
				VkPhysicalDevice physical = d->getPhysicalDevice();
				VkDevice logical = d->getLogicalDevice();
				VkDestroyer(VkBuffer) staging_buffer;

				data = std::vector<float>(m_dataSize);
				InitVkDestroyer(logical, staging_buffer);
				if (!LavaCake::Core::CreateBuffer(logical, sizeof(float) * m_dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, *staging_buffer)) {

				}

				VkDestroyer(VkDeviceMemory) memory_object;
				InitVkDestroyer(logical, memory_object);
				if (!LavaCake::Core::AllocateAndBindMemoryObjectToBuffer(physical, logical, *staging_buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, *memory_object)) {

				}


				
				cmdBuff.beginRecord();

				LavaCake::Core::SetBufferMemoryBarrier(cmdBuff.getHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { { *m_buffer, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });

				LavaCake::Core::CopyDataBetweenBuffers(cmdBuff.getHandle(), *m_buffer, *staging_buffer, { { 0, 0, sizeof(float) * m_dataSize } });

				LavaCake::Core::SetBufferMemoryBarrier(cmdBuff.getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { { *m_buffer, VK_ACCESS_TRANSFER_WRITE_BIT, 0, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });

				cmdBuff.endRecord();

				if (!LavaCake::Core::SubmitCommandBuffersToQueue(queue->getHandle(), {}, { cmdBuff.getHandle() }, {}, cmdBuff.getFence())) {

				}

				cmdBuff.wait(UINT32_MAX);
				cmdBuff.resetFence();
				void* local_pointer;
				VkResult result = vkMapMemory(logical, *memory_object, 0, sizeof(float) * m_dataSize, 0, &local_pointer);
				if (VK_SUCCESS != result) {
					//std::cout << "Could not map memory object." << std::endl;
					//return false;
				}
				std::memcpy(&data[0], local_pointer, static_cast<size_t>(sizeof(float) * m_dataSize));
				vkUnmapMemory(logical, *memory_object);
			}




    protected:

      VkDestroyer(VkBuffer)																m_buffer;
      VkDestroyer(VkDeviceMemory)													m_bufferMemory;
      VkDestroyer(VkBufferView)														m_bufferView;

			uint32_t																						m_dataSize = 0;
    };


    
  }
}