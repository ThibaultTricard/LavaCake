#include "UniformBuffer.h"


namespace LavaCake {
	namespace Framework {

		void UniformBuffer::end() {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
      VkPhysicalDeviceProperties p;
      vkGetPhysicalDeviceProperties(d->getPhysicalDevice(), &p);
      VkDeviceSize padding = p.limits.nonCoherentAtomSize - m_data.size() % p.limits.nonCoherentAtomSize;
      
      //adding empty value at the end of the buffer to match the atomic size of a buffer;
      m_data.resize(m_data.size()+padding/sizeof(std::byte),  std::byte{0});

      VkDeviceSize bufferSize = m_data.size();
      m_stagingBuffer.allocate(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      m_buffer.allocate(bufferSize, (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			copyToStageMemory(true);
		}

		void UniformBuffer::update(CommandBuffer& commandBuffer, bool all) {
			copyToStageMemory(all);

			m_buffer.setAccess(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

			std::vector<VkBufferCopy> regions = {
				{
					0,                        // VkDeviceSize     srcOffset
					0,                        // VkDeviceSize     dstOffset
          m_data.size()             // VkDeviceSize     size
				}
			};

      // Idea : template copyToBuffer to avoid memory allocation (or just take a span as input)
      // then we can use an std::array for regions above
			m_stagingBuffer.copyToBuffer(commandBuffer, m_buffer, regions);

			m_buffer.setAccess(commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT);
		}

		VkBuffer& UniformBuffer::getHandle() {
			return m_buffer.getHandle();
		};

    void UniformBuffer::copyToStageMemory(bool /*all*/) {
      m_stagingBuffer.write(m_data);
		}

    void UniformBuffer::addArray(const std::string& name, std::byte* data, unsigned int size) {
      uint32_t offset_data = (uint32_t)m_data.size();
      auto [it, inserted] = m_variableNames.try_emplace(std::string(name), offset_data, size);
      if (!inserted) [[unlikely]] {
        ErrorCheck::setError("The variable allready exist in this UniformBuffer",1);
        return;
      }

      m_data.resize(offset_data+size);
      std::memcpy(m_data.data()+offset_data, data, size);
		}

    void UniformBuffer::setArray(const std::string& name, std::byte* data, unsigned int size) {
      if (auto it = m_variableNames.find(name); it != m_variableNames.end()) [[likely]] {
        auto [offset_data, size_data] = it->second;
        if (size_data!=size) [[unlikely]] {
          ErrorCheck::setError("The new value does not match the type of the one currently stored in this UniformBuffer",1);
          return;
        }

        std::memcpy(m_data.data()+offset_data, data, size);

  //			if (m_modified.size() > 0) {
  //				m_modified[i] = true;
  //			}
        return;
      }

      ErrorCheck::setError("The variable does not exist in this UniformBuffer",1);
		}
	}
}
