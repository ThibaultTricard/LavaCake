#include "VertexBuffer.h"
#include "CommandBuffer.h"
namespace LavaCake {
	namespace Framework {

		VertexBuffer::VertexBuffer(std::vector <LavaCake::Helpers::Mesh::Mesh*> m, std::vector<int> dataDescription, uint32_t binding, VkVertexInputRate inputRate ) {
			m_meshs = m;

			
			uint32_t offset = 0;
			for (uint32_t i = 0; i < dataDescription.size(); i++) {
				VkFormat f = VK_FORMAT_UNDEFINED;
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

		void VertexBuffer::allocate(VkQueue& queue, VkCommandBuffer& commandBuffer) {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logicalDevice = d->getLogicalDevice();
			VkPhysicalDevice physicalDevice = d->getPhysicalDevice();

			m_vertices = new std::vector<float>(m_meshs[0]->Data);
			m_indices = new std::vector<uint16_t>(m_meshs[0]->index);
			m_indexed = m_meshs[0]->indexed;
			for (unsigned int i = 1; i < m_meshs.size(); i++) {
				if (m_indexed) {
					for (size_t j = 0; j < m_meshs[i]->index.size(); j++) {
						m_indices->push_back(m_meshs[i]->index[j] + m_vertices->size());
					}
				}
				m_vertices->insert(m_vertices->end(), m_meshs[i]->Data.begin(), m_meshs[i]->Data.end());
			}

			if (m_vertices->size() == 0)return;

			InitVkDestroyer(logicalDevice, m_buffer);
			InitVkDestroyer(logicalDevice, m_bufferMemory);
			if (!Buffer::CreateBuffer(logicalDevice, sizeof(m_vertices[0]) * m_vertices->size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, *m_buffer)) {
				ErrorCheck::setError("Can't create vertices buffer");
			}
			
			if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, *m_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_bufferMemory)) {
				ErrorCheck::setError("Can't allocate vertices buffer");
			}
			if (!Memory::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physicalDevice, logicalDevice, sizeof(float) * m_vertices->size(), &(*m_vertices)[0], *m_buffer, 0, 0,
				VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, queue, commandBuffer, {})) {
				ErrorCheck::setError("Can't update buffer memory");
			}

			if (m_indexed) {

				InitVkDestroyer(logicalDevice, m_indexBuffer);
				InitVkDestroyer(logicalDevice, m_indexBufferMemory);
				if (!Buffer::CreateBuffer(logicalDevice, sizeof(m_indices[0]) * m_indices->size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, *m_indexBuffer)) {
					ErrorCheck::setError("Can't create vertices buffer");
				}
				
				if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, *m_indexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_indexBufferMemory)) {
					ErrorCheck::setError("Can't allocate vertices buffer");
				}
				if (!Memory::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physicalDevice, logicalDevice, sizeof(uint16_t) * m_indices->size(), &(*m_indices)[0], *m_indexBuffer, 0, 0,
					VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, queue, commandBuffer, {})) {
					ErrorCheck::setError("Can't update buffer memory");
				}

			}
		}

		std::vector<LavaCake::Helpers::Mesh::Mesh*> VertexBuffer::getMeshs() {
			return m_meshs;
		}

		VkBuffer& VertexBuffer::getBuffer() {
			return *m_buffer;
		}

		VkBuffer& VertexBuffer::getIndexBuffer() {
			return *m_indexBuffer;
		}

		std::vector<VkVertexInputAttributeDescription>& VertexBuffer::getAttributeDescriptions() {
			return m_attributeDescriptions;
		}

		std::vector<VkVertexInputBindingDescription>& VertexBuffer::getBindingDescriptions() {
			return m_bindingDescriptions;
		}

		bool VertexBuffer::isIndexed() {
			return m_indexed;
		};

		void VertexBuffer::swapMeshes(std::vector <LavaCake::Helpers::Mesh::Mesh*>				meshes) {
			m_meshs = meshes;
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logicalDevice = d->getLogicalDevice();
			if (m_indexed) {
				Buffer::DestroyBuffer(logicalDevice, *m_indexBuffer);
				Memory::FreeMemoryObject(logicalDevice, *m_indexBufferMemory);
			}
			Buffer::DestroyBuffer(logicalDevice, *m_buffer);
			Memory::FreeMemoryObject(logicalDevice, *m_bufferMemory);
		};
	}
}