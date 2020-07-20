#include "VertexBuffer.h"
namespace LavaCake {
	namespace Framework {

		VertexBuffer::VertexBuffer(std::vector <LavaCake::Helpers::Mesh::Mesh*> m, std::vector<int> dataDescription, uint32_t binding, VkVertexInputRate inputRate ) {
			m_vertices = new std::vector<float>(m[0]->Data);
			for (unsigned int i = 1; i < m.size(); i++) {
				m_vertices->insert(m_vertices->end(), m[i]->Data.begin(), m[i]->Data.end());
			}
			m_meshs = m;

			uint32_t offset = 0;
			for (uint32_t i = 0; i < dataDescription.size(); i++) {
				VkFormat f;
				if (dataDescription[i] == 1) {
					f = VK_FORMAT_R32_SFLOAT;
				}
				else if (dataDescription[i] == 2) {
					f = VK_FORMAT_R32G32_SFLOAT;
				}
				else if (dataDescription[i] == 3) {
					f = VK_FORMAT_R32G32B32_SFLOAT;
				}
				else if (dataDescription[i] == 4) {
					f = VK_FORMAT_R32G32B32A32_SFLOAT;
				}

				m_attributeDescriptions.push_back({
					i,
					binding,
					f,
					uint32_t(offset * sizeof(float))
					});
				offset += dataDescription[i];
			}
			m_bindingDescriptions.push_back(
				{
							binding,
							uint32_t(offset * sizeof(float)),
							inputRate
				});
		};

		void VertexBuffer::allocate(LavaCake::Framework::Queue& queue, VkCommandBuffer& commandBuffer) {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logicalDevice = d->getLogicalDevice();
			VkPhysicalDevice physicalDevice = d->getPhysicalDevice();
			InitVkDestroyer(logicalDevice, m_buffer);
			if (!Buffer::CreateBuffer(logicalDevice, sizeof(m_vertices[0]) * m_vertices->size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, *m_buffer)) {
				ErrorCheck::setError("Can't create vertices buffer");
			}
			InitVkDestroyer(logicalDevice, m_bufferMemory);
			if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, *m_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_bufferMemory)) {
				ErrorCheck::setError("Can't allocate vertices buffer");
			}
			if (!Memory::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physicalDevice, logicalDevice, sizeof(float) * m_vertices->size(), &(*m_vertices)[0], *m_buffer, 0, 0,
				VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, queue.getHandle(), commandBuffer, {})) {
				ErrorCheck::setError("Can't update buffer memory");
			}
		}

		std::vector<LavaCake::Helpers::Mesh::Mesh*> VertexBuffer::getMeshs() {
			return m_meshs;
		}

		VkBuffer& VertexBuffer::getBuffer() {
			return *m_buffer;
		}

		std::vector<VkVertexInputAttributeDescription>& VertexBuffer::getAttributeDescriptions() {
			return m_attributeDescriptions;
		}

		std::vector<VkVertexInputBindingDescription>& VertexBuffer::getBindingDescriptions() {
			return m_bindingDescriptions;
		}
	}
}