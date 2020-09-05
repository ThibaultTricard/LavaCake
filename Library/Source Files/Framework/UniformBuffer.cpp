#include "UniformBuffer.h"
namespace LavaCake {
	namespace Framework {

		void UniformBuffer::end() {
			m_modified = std::vector<bool>(m_variables.size());
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();

			for (uint32_t i = 0; i < m_variables.size(); i++) {
				VkDeviceSize s = sizeof(m_variables[i][0]) * m_variables[i].size();
				m_typeSizeOffset.push_back(std::pair<VkDeviceSize, VkDeviceSize>(s, m_bufferSize));
				m_bufferSize += s;
			}

			InitVkDestroyer(logical, m_stagingBuffer);
			if (!LavaCake::Buffer::CreateBuffer(logical, m_bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *m_stagingBuffer)) {
				ErrorCheck::setError("Can't create staging buffer");
			}
			InitVkDestroyer(logical, m_stagingBufferMemory);
			if (!LavaCake::Buffer::AllocateAndBindMemoryObjectToBuffer(physical, logical, *m_stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, *m_stagingBufferMemory)) {
				ErrorCheck::setError("Can't allocate staging buffer memory");
			}

			InitVkDestroyer(logical, m_buffer);
			InitVkDestroyer(logical, m_bufferMemory);
			if (!Buffer::CreateUniformBuffer(physical, logical, m_bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				*m_buffer, *m_bufferMemory)) {
				ErrorCheck::setError("Can't create uniform buffer");
			}
			copyToStageMemory(true);
		}

		void UniformBuffer::update(VkCommandBuffer& commandBuffer, bool all) {
			copyToStageMemory(all);

			Buffer::BufferTransition pre_transfer_transition = {
				*m_buffer,                      // VkBuffer         Buffer
				VK_ACCESS_UNIFORM_READ_BIT,   // VkAccessFlags    CurrentAccess
				VK_ACCESS_TRANSFER_WRITE_BIT, // VkAccessFlags    NewAccess
				VK_QUEUE_FAMILY_IGNORED,      // uint32_t         CurrentQueueFamily
				VK_QUEUE_FAMILY_IGNORED       // uint32_t         NewQueueFamily
			};
			Buffer::SetBufferMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { pre_transfer_transition });

			std::vector<VkBufferCopy> regions = {
				{
					0,                        // VkDeviceSize     srcOffset
					0,                        // VkDeviceSize     dstOffset
					m_bufferSize                // VkDeviceSize     size
				}
			};
			Memory::CopyDataBetweenBuffers(commandBuffer, *m_stagingBuffer, *m_buffer, regions);

			Buffer::BufferTransition post_transfer_transition = {
				*m_buffer,                      // VkBuffer         Buffer
				VK_ACCESS_TRANSFER_WRITE_BIT, // VkAccessFlags    CurrentAccess
				VK_ACCESS_UNIFORM_READ_BIT,   // VkAccessFlags    NewAccess
				VK_QUEUE_FAMILY_IGNORED,      // uint32_t         CurrentQueueFamily
				VK_QUEUE_FAMILY_IGNORED       // uint32_t         NewQueueFamily
			};
			Buffer::SetBufferMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, { post_transfer_transition });
		}

		VkBuffer& UniformBuffer::getHandle() {
			return *m_buffer;
		};

		void UniformBuffer::copyToStageMemory(bool all) {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			std::vector<int> variable;
			VkDeviceSize size = 0;
			for (uint32_t i = 0; i < m_variables.size(); i++) {
				size += m_typeSizeOffset[i].first;
				for (uint32_t j = 0; j < m_variables[i].size(); j++) {
					variable.push_back(m_variables[i][j]);
				}
			}
			if (!Memory::MapUpdateAndUnmapHostVisibleMemory(logical, *m_stagingBufferMemory, 0, size, &variable[0], true, nullptr)) {
				ErrorCheck::setError("Can't map host visible memomry");
			}
		}

		void UniformBuffer::addArray(std::string name, std::vector<int>& value) {
			if (m_variableNames.find(name) != m_variableNames.end()) {
				ErrorCheck::setError("The variable allready exist in this UniformBuffer");
				return;
			}
			int i = int(m_variables.size());
			m_variables.push_back(value);
			m_variableNames.insert(std::pair<std::string, int>(name, i));
		}

		void UniformBuffer::setArray(std::string name, std::vector<int>& value) {
			if (m_variableNames.find(name) == m_variableNames.end()) {
				ErrorCheck::setError("The variable does not exist in this UniformBuffer");
				return;
			}
			int i = m_variableNames[name];
			std::vector<int> v = m_variables[i];
			if (v.size() != value.size()) {
				ErrorCheck::setError("The new value does not match the type of the one currently stored in this UniformBuffer");
				return;
			}

			m_variables[i] = value;
			if (m_modified.size() > 0) {
				m_modified[i] = true;
			}
		}
	}
}