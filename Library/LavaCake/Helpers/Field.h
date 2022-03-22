#pragma once
#include "helpers.h"
#include "ABBox.h"
#include "Math/basics.h"

namespace LavaCake {
  namespace Helpers {
  
  /**
   *Class Field2D :
   *\brief : A virtual class that represent any 2D field of value
   *\tparam T  the type the Field will hold
   */
  template <typename T>
  class Field2D{
  public:
    /**
     \brief sample the field at a postion
     \param pos a vec2f representing the sample position
     \return the value of the field at the position pos
     */
    virtual T sample(vec2f pos) = 0;
  };

  /**
   *Class  Field2DGrid :
   *\brief A class that represent a 2D field sampled on a regular grid
   *\tparam T  the type the Field will hold
   */
  template <typename T>
  class Field2DGrid : public Field2D<T>{
    public:
    
    /**
     *\brief Create a 2D field
     *\param data a std::vector of data
     *\param width the width of the grid
     *\param height the height of the grid
     *\param boundingbox the bounding box of the field in the domain
     *\param interpolate [optional]  a funtion pointer to an interpolation function for the type T
     */
    Field2DGrid(std::vector<T>& data,
                uint32_t width,
                uint32_t height,
                ABBox<2> boundingbox,
                T (*interpolate)(T&, T&, float) = nullptr
    ): Field2D<T>(){
      
      m_fields = data;
      m_width = width;
      m_height = height;
      m_boundingbox = boundingbox;
      m_interpolate = interpolate;
    }
    
    /**
     \brief sample the field at a postion
     \param pos a vec2f representing the sample position
     \return the value of the field at the position pos
     */
    T sample(vec2f pos) override{
      vec2f X = (pos - m_boundingbox.A()) / (m_boundingbox.diag());
      X = X * vec2f({float(m_width-1), float(m_height-1)});
      X = vec2f({fmax(0.0f, X[0]), fmax(0.0f, X[1])});
      
      vec2u U = vec2u({uint32_t(X[0]),uint32_t(X[1])});
      vec2f R = X - vec2f({float(U[0]),float(U[1])});
      
      
      if(U[0] >= m_width-1){
        U[0] = m_width-2;
        R[0] = 1.0f;
      }
      if(U[1] >= m_height-1){
        U[1] = m_height-2;
        R[1] = 1.0f;
      }
      T A,B,C,D;
      A = m_fields[U[0]     +  U[1]    *m_width];
      B = m_fields[U[0]+1   +  U[1]    *m_width];
      C = m_fields[U[0]     + (U[1]+1) *m_width];
      D = m_fields[U[0]+1   + (U[1]+1) *m_width];
      

      if (m_interpolate == nullptr) {
        T AB = B * R[0] + A * (1.0f - R[0]);
        T CD = D * R[0] + C * (1.0f - R[0]);
        return CD * R[1] + AB * (1.0f - R[1]);
      }
      T AB = m_interpolate(A, B, R[0]);
      T CD = m_interpolate(C, D, R[0]);
      return m_interpolate(AB, CD, R[1]);
      
    }
    
    
    std::vector<T> getRawField(){return m_fields;}
    vec2u getDimension(){return{m_width,m_height};}
    
    /**
     \brief get the bounding box of the field
     \return a bounding box 2D
     */
    ABBox<2> getABBox(){return m_boundingbox;}
    
    private :
    uint32_t        m_width;
    uint32_t        m_height;
    std::vector<T>  m_fields;
    ABBox<2>        m_boundingbox;

    T (*m_interpolate)(T&, T&, float);
  };


  /**
   *Class Field3D :
   *\brief : A virtual class that represent any 3D field of value
   *\tparam T  the type the Field will hold
   */
  template <typename T>
  class Field3D{
  public:
    /**
     \brief sample the field at a postion
     \param pos a vec3f representing the sample position
     \return the value of the field at the position pos
     */
    virtual T sample(vec3f pos) = 0;
  };
  
  
  /**
   *Class  Field3DGrid :
   *\brief A class that represent a 3D field sampled on a regular grid
   *\tparam T  the type the Field will hold
   */
  template <typename T>
  class Field3DGrid : public Field3D<T>{
  public:
    
    /**
     *\brief Create a 3D field
     *\param data a std::vector of data
     *\param width the width of the grid
     *\param height the height of the grid
     *\param dpeth  the depth of the grid
     *\param boundingbox the bounding box of the field in the domain
     *\param interpolate [optional]  a funtion pointer to an interpolation function for the type T
     */
    Field3DGrid(std::vector<T>& data, uint32_t width, uint32_t height, uint32_t depth, ABBox<3> boundingbox, T (*interpolate)(T&, T&, float) = nullptr){
      m_fields = data;
      m_width = width;
      m_height = height;
      m_depth = depth;
      m_boundingbox = boundingbox;
      m_interpolate = interpolate;
    }
    
    /**
     \brief sample the field at a postion
     \param pos a vec3f representing the sample position
     \return the value of the field at the position pos
     */
    T sample(vec3f pos) override{
      vec3f X = (pos - m_boundingbox.A()) / (m_boundingbox.B() - m_boundingbox.A());
      X = X * vec3f({float(m_width-1), float(m_height-1), float(m_depth-1)});
      
      X = vec3f({fmax(0.0f, X[0]), fmax(0.0f, X[1]), fmax(0.0f, X[2])});
      
      vec3u U = vec3u({uint32_t(X[0]),uint32_t(X[1]), uint32_t(X[2])});
      vec3f R = X - vec3f({float(U[0]),float(U[1]),float(U[2])});
      
      
      if(U[0] >= m_width-1){
        U[0] = m_width-2;
        R[0] = 1.0f;
      }
      if(U[1] >= m_height-1){
        U[1] = m_height-2;
        R[1] = 1.0f;
      }
      if(U[2] >= m_depth-1){
        U[2] = m_depth-2;
        R[2] = 1.0f;
      }
      
      T A,B,C,D,E,F,G,H;
      A = m_fields[U[0]   +  U[1]    *m_width + U[2]    *m_width*m_height];
      B = m_fields[U[0]+1 +  U[1]    *m_width + U[2]    *m_width*m_height];
      C = m_fields[U[0]   + (U[1]+1) *m_width + U[2]    *m_width*m_height];
      D = m_fields[U[0]+1 + (U[1]+1) *m_width + U[2]    *m_width*m_height];
      E = m_fields[U[0]   +  U[1]    *m_width + (U[2]+1)*m_width*m_height];
      F = m_fields[U[0]+1 +  U[1]    *m_width + (U[2]+1)*m_width*m_height];
      G = m_fields[U[0]   + (U[1]+1) *m_width + (U[2]+1)*m_width*m_height];
      H = m_fields[U[0]+1 + (U[1]+1) *m_width + (U[2]+1)*m_width*m_height];

      if (m_interpolate == nullptr) {
        T AB = B * R[0] + A * (1.0f - R[0]);
        T CD = D * R[0] + C * (1.0f - R[0]);

        T ABCD = CD * R[1] + AB * (1.0f - R[1]);

        T EF = F * R[0] + E * (1.0f - R[0]);
        T GH = H * R[0] + G * (1.0f - R[0]);

        T EFGH = GH * R[1] + EF * (1.0f - R[1]);

        return EFGH * R[2] + ABCD * (1.0f - R[2]);
      }

      T AB = m_interpolate(A, B, R[0]);
      T CD = m_interpolate(A, B, R[0]);
      T ABCD = m_interpolate(AB, CD, R[1]);

      T EF = m_interpolate(E, F, R[0]);
      T GH = m_interpolate(G, H, R[0]);
      T EFGH = m_interpolate(EF, GH, R[1]);
      return m_interpolate(ABCD, EFGH, R[2]);

    }
    
    std::vector<T> getRawField(){return m_fields;};
    vec3u getDimension(){return{m_width,m_height,m_depth};};
    
    /**
     \brief get the bounding box of the field
     \return a bounding box 3D
     */
    ABBox<3> getABBox(){return m_boundingbox;};
    
    private :
    uint32_t        m_width;
    uint32_t        m_height;
    uint32_t        m_depth;
    std::vector<T>  m_fields;
    ABBox<3>        m_boundingbox;

    T (*m_interpolate)(T&, T&, float);
  };
  
  
  }
}
