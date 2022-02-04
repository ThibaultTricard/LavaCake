#pragma once
#include "format.h"
namespace LavaCake {
  namespace Geometry {
  
  /**
   *\brief Class Mesh_t : a Virtual class that represent a mesh
   */
    class Mesh_t {
    protected:
      /**
       *\brief Class Mesh : a Virtual class that represent a mesh
       */
      Mesh_t() {};

    public :
      
      virtual ~Mesh_t() = default;
      
      /**
       *\brief Add a vertex in the mesh
       *\param vertex a list of float that represent the vertex to be add in the mesh
       */
      virtual void appendVertex(std::vector<float> vertex) = 0;
      
      /**
       *\brief Add an index in the mesh
       *\param index the index to add in the mesh
       */
      virtual void appendIndex(uint32_t index) = 0;

      /**
       *\brief Return the number of float in a vertex
       */
      virtual size_t vertexSize() const = 0;
      
      /**
       *\brief Return the vulkan input attribute description of the mesh format
       */
      virtual std::vector<VkVertexInputAttributeDescription> VkDescription() const = 0;

      /**
       *\brief Return whether or not the mesh is indexed
       */
      virtual bool isIndexed() const = 0;

      /**
       *\brief Return all the vertices of the mesh in an array of float
       */
      virtual std::vector<float>& vertices() = 0;

      /**
       *\brief Return all the vertices of the mesh in an array of float
       */
      virtual const std::vector<float>& vertices() const = 0;
      
      /**
       *\brief Return all the indices of the mesh in an array of unsigned int 32 bits
       */
      virtual std::vector<uint32_t>& indices() = 0;

      /**
       *\brief Return all the indices of the mesh in an array of unsigned int 32 bits
       */
      virtual const std::vector<uint32_t>& indices() const = 0;

      /**
       *\brief Return the topology of the mesh
       */
      virtual topology getTopology() const = 0;
      
      /**
       *\brief Return the vertex format of the mesh
       */
      virtual vertexFormat getFormat() const = 0;


    };

  /**
   *\brief Class Mesh : a class that represent an un-indexed mesh
   *\tparam T The topology of the mesh
   */
    template <topology T>
    class Mesh : public Mesh_t {
    protected :
      std::vector<float>      m_vertices;
      std::vector<uint32_t>   m_indices;
      vertexFormat            m_format;
      size_t                  m_vertexSize;
      bool                    m_indexed = false;

    public:
      virtual ~Mesh() = default;

      Mesh(vertexFormat format) {
        m_vertexSize = format.size();
        m_format = format;
      }

      Mesh(std::vector<float>& vertices, vertexFormat format) {
        m_vertexSize = format.size();
        m_format = format;
        m_vertices = vertices;
      }

      virtual void appendVertex(std::vector<float> vertex) override {
        if (vertex.size() == m_vertexSize)
          m_vertices.insert(m_vertices.end(), vertex.begin(), vertex.end());
      }

      virtual void appendIndex(uint32_t index) override {
      }

      virtual size_t vertexSize() const override {
        return m_vertexSize;
      }

      virtual std::vector<VkVertexInputAttributeDescription> VkDescription() const override {
        return m_format.VkDescription();
      }

      virtual bool isIndexed() const override {
        return m_indexed;
      }

      virtual std::vector<float>& vertices() override {
        return m_vertices;
      }

      virtual const std::vector<float>& vertices() const override {
        return m_vertices;
      }

      virtual std::vector<uint32_t>& indices() override {
        return m_indices;
      }

      virtual const std::vector<uint32_t>& indices() const override {
        return m_indices;
      }

      virtual topology getTopology() const override {
        return T;
      }
      
      virtual vertexFormat getFormat() const override {
        return m_format;
      };

    };

  /**
   *\brief Class IndexedMesh : a class that represent an indexed mesh
   *\tparam T The topology of the mesh
   */
    template <topology T>
    class IndexedMesh : public Mesh<T> {
    public :
      IndexedMesh(vertexFormat format) : Mesh<T>(format) {
        this->m_indexed = true;
      }


      IndexedMesh(std::vector<float>& vertices, std::vector<uint32_t>& indices, vertexFormat format) : Mesh<T>(format) {
        this->m_vertexSize = format.size();
        this->m_format = format;
        this->m_vertices = vertices;
        this->m_indices = indices;
        this->m_indexed = true;
      }


      virtual void appendIndex(uint32_t index) override {
        this->m_indices.push_back(index);
      }

     };

    


    typedef Mesh<POINT> PointCloud;
    typedef Mesh<LINE> LineMesh;
    typedef IndexedMesh<LINE> LineIndexedMesh;
    typedef Mesh<TRIANGLE> TriangleMesh;
    typedef IndexedMesh<TRIANGLE> TriangleIndexedMesh;



    static Mesh_t* generateQuad(bool addUV = false) {
      Mesh_t* m = new TriangleIndexedMesh(P3UV);

      if (addUV) {
        m->appendVertex({ -1.0,-1.0,0.0,0.0,0.0 });
        m->appendVertex({ -1.0, 1.0,0.0,0.0,1.0 });
        m->appendVertex({ 1.0, 1.0,0.0,1.0,1.0 });
        m->appendVertex({ 1.0,-1.0,0.0,1.0,0.0 });
      }
      else {
        m->appendVertex({ -1.0,-1.0,0.0 });
        m->appendVertex({ -1.0, 1.0,0.0 });
        m->appendVertex({ 1.0, 1.0,0.0 });
        m->appendVertex({ 1.0,-1.0,0.0});
      }

      m->appendIndex(0);
      m->appendIndex(1);
      m->appendIndex(2);

      m->appendIndex(2);
      m->appendIndex(3);
      m->appendIndex(0);

      return m;
    }

  }
}
