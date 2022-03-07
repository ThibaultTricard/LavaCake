#include "UniformBuffer.h"


namespace LavaCake {
    namespace Framework {

    void UniformBuffer::end() {
      //allocate both the buffer and the staging buffer
      m_stagingBuffer = std::make_shared<Buffer>(m_data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      m_buffer = std::make_shared<Buffer>(m_data.size(), (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    void UniformBuffer::update(CommandBuffer& commandBuffer) {
      copyToStageMemory(true);

			m_buffer->setAccess(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

			std::vector<VkBufferCopy> regions = {
				{
					0,                        // VkDeviceSize     srcOffset
					0,                        // VkDeviceSize     dstOffset
          m_data.size()             // VkDeviceSize     size
				}
			};

      // Idea : template copyToBuffer to avoid memory allocation (or just take a span as input)
      // then we can use an std::array for regions above
			m_stagingBuffer->copyToBuffer(commandBuffer, *m_buffer, regions);

			m_buffer->setAccess(commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT);
		}

		const VkBuffer& UniformBuffer::getHandle() const {
			return m_buffer->getHandle();
		};


    void UniformBuffer::copyToStageMemory(bool /*all*/) {
      m_stagingBuffer->write(m_data);
		}
	}
}
