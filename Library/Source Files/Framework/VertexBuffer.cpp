#include "VertexBuffer.h"
#include "CommandBuffer.h"

namespace LavaCake {
	namespace Framework {

		VertexBuffer::VertexBuffer(std::vector<LavaCake::Geometry::Mesh_t*> m, uint32_t binding, bool Computable ,VkVertexInputRate inputRate) {

			m_topology = m[0]->getTopology();
			uint32_t offset = (uint32_t)m[0]->vertexSize();
			
			m_attributeDescriptions = m[0]->VkDescription();

			for (size_t t = 0; t < m_attributeDescriptions.size(); t++) {
				m_attributeDescriptions[t].binding = binding;
			}


			m_bindingDescriptions.push_back(
				{
							binding,
							uint32_t(offset * sizeof(float)),
							inputRate
				});

			swapMeshes(m);
		};

		void VertexBuffer::allocate(VkQueue& queue, VkCommandBuffer& commandBuffer) {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logicalDevice = d->getLogicalDevice();
			VkPhysicalDevice physicalDevice = d->getPhysicalDevice();
			

			if (m_vertices.size() == 0)return;

			InitVkDestroyer(logicalDevice, m_buffer);
			InitVkDestroyer(logicalDevice, m_bufferMemory);
			if (!Buffer::CreateBuffer(logicalDevice, sizeof(m_vertices[0]) * m_vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, *m_buffer)) {
				ErrorCheck::setError("Can't create vertices buffer");
			}
			
			if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, *m_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_bufferMemory)) {
				ErrorCheck::setError("Can't allocate vertices buffer");
			}
			if (!Memory::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physicalDevice, logicalDevice, sizeof(float) * m_vertices.size(), &(m_vertices)[0], *m_buffer, 0, 0,
				VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, queue, commandBuffer, {})) {
				ErrorCheck::setError("Can't update buffer memory");
			}

			if (m_indexed) {

				InitVkDestroyer(logicalDevice, m_indexBuffer);
				InitVkDestroyer(logicalDevice, m_indexBufferMemory);
				if (!Buffer::CreateBuffer(logicalDevice, sizeof(m_indices[0]) * m_indices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, *m_indexBuffer)) {
					ErrorCheck::setError("Can't create vertices buffer");
				}
				
				if (!Buffer::AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, *m_indexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_indexBufferMemory)) {
					ErrorCheck::setError("Can't allocate vertices buffer");
				}
				if (!Memory::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physicalDevice, logicalDevice, sizeof(uint32_t) * m_indices.size(), &(m_indices)[0], *m_indexBuffer, 0, 0,
					VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, queue, commandBuffer, {})) {
					ErrorCheck::setError("Can't update buffer memory");
				}

			}
		}

		
		
		VkBuffer& VertexBuffer::getHandle() {
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

		
		void VertexBuffer::swapMeshes(std::vector<LavaCake::Geometry::Mesh_t*>				m) {
			if (m_topology == m[0]->getTopology()) {
				m_vertices = std::vector<float>(m[0]->vertices());
				m_indices = std::vector<uint32_t>(m[0]->indices());

				m_indexed = m[0]->isIndexed();
				for (unsigned int i = 1; i < m.size(); i++) {
					if (m_indexed) {
						for (size_t j = 0; j < m[i]->indices().size(); j++) {
							m_indices.push_back(m[i]->indices()[j] + uint32_t(m_vertices.size()));
						}
					}
					m_vertices.insert(m_vertices.end(), m[i]->vertices().begin(), m[i]->vertices().end());
				}
			}

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