#pragma once
#include "format.h"
namespace LavaCake {
  namespace Geometry {

    class Mesh_t {
    public :
      Mesh_t() {};

      virtual void appendVertex(std::vector<float> vertex) = 0;

      virtual void appendIndex(uint32_t index) = 0;

      virtual size_t vertexSize() = 0;

      virtual std::vector<VkVertexInputAttributeDescription> VkDescription() = 0;

      virtual bool isIndexed() = 0;

      virtual std::vector<float>& vertices() = 0;
      virtual  std::vector<uint32_t>& indices() = 0;
      virtual  topology getTopology() = 0;
    };

    template <topology T>
    class Mesh : public Mesh_t {
    protected :
      std::vector<float>      m_vertices;
      std::vector<uint32_t>   m_indices;
      vertexFormat            m_format;
      size_t                  m_vertexSize;
      bool                    m_indexed = false;

    public:

      Mesh(vertexFormat format) {
        m_vertexSize = format.size();
        m_format = format;
      }


      virtual void appendVertex(std::vector<float> vertex) override {
        if (vertex.size() == m_vertexSize)
          m_vertices.insert(m_vertices.end(), vertex.begin(), vertex.end());
      }

      virtual void appendIndex(uint32_t index) override {
        m_indices.push_back(index);
      }

      virtual size_t vertexSize() override {
        return m_vertexSize;
      }

      virtual std::vector<VkVertexInputAttributeDescription> VkDescription() override {
        return m_format.VkDescription();
      }

      virtual bool isIndexed() override {
        return m_indexed;
      }

      virtual std::vector<float>& vertices() override {
        return m_vertices;
      }

      virtual std::vector<uint32_t>& indices() override {
        return m_indices;
      }

      virtual topology getTopology() override {
        return T;
      }
      
    };

    template <topology T>
    class IndexedMesh : public Mesh<T> {
    public :
      IndexedMesh(vertexFormat format) : Mesh(format) {
        m_indexed = true;
      }
    };




    typedef Mesh<POINT> PointCloud;
    typedef Mesh<LINE> LineMesh;
    typedef IndexedMesh<LINE> LineIndexedMesh;
    typedef Mesh<TRIANGLE> TriangleMesh;
    typedef IndexedMesh<TRIANGLE> TriangleIndexedMesh;

  }
}