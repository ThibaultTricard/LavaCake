#pragma once
#include "AllHeaders.h"
#include "Queue.h"
#include "Device.h"
#include <LavaCake/Geometry/mesh.h>
#include "Buffer.h"

namespace LavaCake {
  namespace Framework {

    class VertexBuffer {
    public:

      VertexBuffer(const Queue& queue, CommandBuffer& cmdBuff, const std::vector<std::shared_ptr<LavaCake::Geometry::Mesh_t>>& m, uint32_t binding = 0, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX, VkBufferUsageFlags otherUsage = VkBufferUsageFlags(0));

      std::shared_ptr<Buffer> getVertexBuffer() const;

      std::shared_ptr<Buffer> getIndexBuffer() const;

      const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const;

      const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const;

      VkPrimitiveTopology primitiveTopology() {
        switch (m_topology)
        {
        case LavaCake::Geometry::POINT:
          return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
          break;
        case LavaCake::Geometry::LINE:
          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
          break;
        case LavaCake::Geometry::TRIANGLE:
          return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
          break;
        default:
          return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
          break;
        }
      }


      VkPolygonMode polygonMode() {
        switch (m_topology)
        {
        case LavaCake::Geometry::POINT:
          return VK_POLYGON_MODE_POINT;
          break;
        case LavaCake::Geometry::LINE:
          return VK_POLYGON_MODE_LINE;
          break;
        case LavaCake::Geometry::TRIANGLE:
          return VK_POLYGON_MODE_FILL;
          break;
        default:
          return VK_POLYGON_MODE_FILL;
          break;
        }
      }

      size_t getIndicesNumber() const {
        return m_indicesSize;
      }

      size_t getVerticiesNumber() const {
        return m_verticesSize / m_stride;
      }

      uint32_t getStrideSize() const {
        return m_stride;
      }

      uint32_t getByteStrideSize() const {
        return m_stride * sizeof(float);
      }

      bool isIndexed() const;

      ~VertexBuffer() {
      }

    private:


      std::vector<VkVertexInputAttributeDescription>                m_attributeDescriptions;
      std::vector<VkVertexInputBindingDescription>                  m_bindingDescriptions;
      std::shared_ptr<Buffer>                                       m_vertexBuffer;
      std::shared_ptr<Buffer>                                       m_indexBuffer;
      uint32_t                                                      m_verticesSize = 0;
      uint32_t                                                      m_indicesSize = 0;
      uint32_t                                                      m_stride = 0;
      bool                                                          m_indexed = false;
      LavaCake::Geometry::topology                                  m_topology;
    };

  }
}
