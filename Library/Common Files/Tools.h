// MIT License
//
// Copyright( c ) 2017 Packt
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Vulkan Cookbook
// ISBN: 9781786468154
// © Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329
//
// Tools

#ifndef TOOLS
#define TOOLS

#include <tuple>
#include "Common.h"

namespace LavaCake {

  using vec2f = std::array<float, 2>;
  using vec2d = std::array<double, 2>;
  using vec2i = std::array<int, 2>;
  using vec2u = std::array<uint32_t, 2>;
  using vec2b = std::array<bool, 2>;

  using vec3f = std::array<float, 3>;
  using vec3d = std::array<double, 3>;
  using vec3i = std::array<int, 3>;
  using vec3u = std::array<uint32_t, 3>;
  using vec3b = std::array<bool, 3>;

  using vec4f = std::array<float, 4>;
  using vec4d = std::array<double, 4>;
  using vec4i = std::array<int, 4>;
  using vec4u = std::array<uint32_t, 3>;
  using vec4b = std::array<bool, 4>;

  using mat2 = std::array<float, 4>;
  using mat3 = std::array<float, 9>;
  using mat4 = std::array<float, 16>;


  template<typename T, unsigned char N>
  std::array<T, N> operator*(const std::array<T, N>& b, const T a) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned char N>
  std::array<T, N> operator*(const T a, const std::array<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned char N>
  std::array<T, N> operator-(const std::array<T, N> a, const std::array<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] - b[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned char N>
  std::array<T, N> operator+(const std::array<T, N> a, const std::array<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] + b[i];
    }
    return std::array<T, N>(d);
  }

  float Deg2Rad(float value);

  bool GetBinaryFileContents(std::string const& filename,
    std::vector<unsigned char>& contents);

  float Dot(vec3f const& left,
    vec3f const& right);

  vec3f Cross(vec3f const& left,
    vec3f const& right);

  vec3f Normalize(vec3f const& vector);

  vec3f operator* (vec3f const& left,
    mat4 const& right);

  bool operator== (vec3f const& left,
    vec3f const& right);

  float Dot(vec3f const & left,
    vec3f const & right );

  vec3f Cross( vec3f const & left,
                 vec3f const & right );

  vec3f Normalize( vec3f const & vector );

  mat4 operator* (mat4 const& left,
    mat4 const& right);

  mat4 inverse(mat4& m);

  mat4 inverse(const mat4& m);
} 






#endif // TOOLS