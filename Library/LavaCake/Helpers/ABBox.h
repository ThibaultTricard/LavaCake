#pragma once
#include <array>
#include "AllHeaders.h"
#include "../Math/basics.h"

namespace LavaCake {
  namespace Helpers {
  /**
   *Class ABBox :
   *\brief Represent a N dimentionals Bounding box
   *\tparam N The number of dimention
   */
    template<uint8_t N, typename T = float>
    class ABBox{
    public:
      
      
      /**
       \brief default constructor
      */
      ABBox(){
        m_min = vec<T, N>();
        m_min.data.fill((T)(INFINITY));
        m_max = vec<T, N>();
        m_max.data.fill((T)(-INFINITY));
        m_diagDirty = true;
      };
      
      /**
       \brief Create a bounding box with it's min and max point
       \param min the min point of the bounding box
       \param max the max point of the bounding box
       */
      ABBox(vec<T,N>& min,vec<T,N>& max){
        m_min = min;
        m_max = max;
        m_diagDirty = true;
      }
      
      /**
       \brief Create a bounding box with it's min and max point
       \param min the min point of the bounding box
       \param max the max point of the bounding box
       */
      ABBox(const vec<T,N>& min,const vec<T,N>& max){
        m_min = min;
        m_max = max;
        m_diagDirty = true;
      }
      
      /**
       \brief get the min point of the bounding box
       \return a std array representing the min point of the bouding box
      */
      vec<T, N> A(){
        return m_min;
      }
      
      /**
       \brief get the max point of the bounding box
       \return a std array representing the max point of the bouding box
       */
      vec<T, N> B(){
        return m_max;
      }
      
      /**
       \brief compute the bounding box diagonal
       \return a std array representing the diagonal of the bounding box
       */
      vec<T, N> diag(){
        if (m_diagDirty) {
          m_diag = m_max - m_min;
        }
        return m_diag;
      }

      /**
       \brief check if a point is contained in the bounding box
       \param  point : a N-dimensional point
       \return true if the point is contained false otherwise
       */
      bool isContained(std::array<T, N>& point) {
        for (uint8_t u = 0; u < N; u++) {
          if (point[u] < m_min[u]) {
            return false;
          }
          if (point[u] > m_max[u]) {
            return false;
          }
        }
        return true;
      }

      /**
       \brief enlarge the bounding box so the point passed is contained
       \param  point : a N-dimensional point
       */
      void addPoint(const vec<T, N>& point) {
        vec<T, N> newMin;
        vec<T, N> newMax;
        for (uint8_t u = 0; u < N; u++) {
          if (point[u] < m_min[u]) {
            newMin[u] = point[u];
            m_diagDirty = true;
          }
          else {
            newMin[u] = m_min[u];
          }
          if (point[u] > m_max[u]) {
            newMax[u] = point[u];
            m_diagDirty = true;
          }
          else {
            newMax[u] = m_min[u];
          }
        }
        m_min = newMin;
        m_max = newMax;
      }


    private:
      
      
      vec<T,N> m_min;
      vec<T,N> m_max;

      vec<T, N> m_diag;
      bool             m_diagDirty =true;
      
    };
  }
}
