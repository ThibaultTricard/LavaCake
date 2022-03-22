#pragma once

#include <cmath>
#include <array>
#include <tuple>

namespace LavaCake {

  using vec2f = std::array<float, 2>;
  using vec2d = std::array<double, 2>;
  using vec2i = std::array<int, 2>;
  using vec2u = std::array<unsigned int, 2>;
  using vec2b = std::array<bool, 2>;

  using vec3f = std::array<float, 3>;
  using vec3d = std::array<double, 3>;
  using vec3i = std::array<int, 3>;
  using vec3u = std::array<unsigned int, 3>;
  using vec3b = std::array<bool, 3>;

  using vec4f = std::array<float, 4>;
  using vec4d = std::array<double, 4>;
  using vec4i = std::array<int, 4>;
  using vec4u = std::array<unsigned int, 4>;
  using vec4b = std::array<bool, 4>;

  using mat2 = std::array<float, 4>;
  using mat3 = std::array<float, 9>;
  using mat4 = std::array<float, 16>;


  template<typename T, unsigned long N>
  std::array<T, N> operator*(const std::array<T, N>& b, const T a) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator*(const T a, const std::array<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator*(const std::array<T, N>& a, const std::array<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator/(const std::array<T, N>& a, const std::array<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] / b[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator/(const std::array<T, N>& b, const T a) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] / a;
    }
    return std::array<T, N>(d);
  }


  template<typename T, unsigned long N>
  std::array<T, N> operator-(const std::array<T, N> a, const std::array<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] - b[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator+(const std::array<T, N> a, const std::array<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] + b[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  T dot(std::array<T, N> const & left,
        std::array<T, N> const & right ){
    T sum = static_cast<T>(0);
    for(unsigned long u = 0; u< N ; u++){
      sum +=  left[u] * right[u];
    }
    return sum;
  };

  template<typename T, unsigned long N>
  std::array<T, N> normalize(std::array<T, N> const vector) {
    float length = dot(vector,vector);
    if (length >0)
      return vector/sqrtf(length);
    return vector;
  }

  float Deg2Rad(float value);

  float Dot(vec3f const& left,
            vec3f const& right) ;

  vec3f cross(vec3f const& left,
              vec3f const& right) ;

  vec3f Cross(vec3f const& left,
              vec3f const& right) ;

  vec3f Normalize(vec3f const& vector) ;


  vec3f operator* (vec3f const& left,
                   mat4 const& right) ;

  vec4f operator* (mat4 const& left,
                   vec4f const& right);

  bool operator== (vec3f const& left,
                   vec3f const& right) ;

  mat4 operator* (mat4 const& left,
                  mat4 const& right) ;

  mat4 inverse(mat4& m) ;

  mat4 inverse(const mat4& m) ;

  mat4 Identity() ;

  mat4 PrepareTranslationMatrix(float x,
                                float y,
                                float z) ;
  mat4 PrepareRotationMatrix(float           angle,
                             vec3f const & axis,
                             float           normalize_axis = 0.0f) ;

  mat4 PrepareScalingMatrix(float x,
                            float y,
                            float z) ;

  mat4 PreparePerspectiveProjectionMatrix(float aspect_ratio,
                                          float field_of_view,
                                          float near_plane,
                                          float far_plane) ;

  mat4 PrepareOrthographicProjectionMatrix(float left_plane,
                                           float right_plane,
                                           float bottom_plane,
                                           float top_plane,
                                           float near_plane,
                                           float far_plane) ;

}
