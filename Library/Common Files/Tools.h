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

  typedef std::array<float, 2> vec2f;
  typedef std::array<double, 2> vec2d;
  typedef std::array<int, 2> vec2i;
  typedef std::array<uint32_t, 2> vec2u;
  typedef std::array<bool, 2> vec2b;


  typedef std::array<float, 3> vec3f;
  typedef std::array<double, 3> vec3d;
  typedef std::array<int, 3> vec3i;
  typedef std::array<bool, 3> vec3b;

  typedef std::array<float, 4> vec4f;
  typedef std::array<double, 4> vec4d;
  typedef std::array<int, 4> vec4i;
  typedef std::array<bool, 4> vec4b;

  using mat2 = std::array<float, 4>;
  using mat3 = std::array<float, 9>;
  using mat4 = std::array<float, 16>;

  bool GetBinaryFileContents( std::string const          & filename,
                              std::vector<unsigned char> & contents );

  float Deg2Rad( float value );

  float Dot(vec3f const & left,
    vec3f const & right );

  vec3f Cross( vec3f const & left,
                 vec3f const & right );

  vec3f Normalize( vec3f const & vector );

  

} // namespace VulkanCookbook


LavaCake::vec3f operator+ (LavaCake::vec3f const& left,
  LavaCake::vec3f const& right);

LavaCake::vec3f operator- (LavaCake::vec3f const& left,
  LavaCake::vec3f const& right);

LavaCake::vec3f operator+ (float const& left,
  LavaCake::vec3f const& right);

LavaCake::vec3f operator- (float const& left,
  LavaCake::vec3f const& right);

LavaCake::vec3f operator+ (LavaCake::vec3f const& left,
  float const& right);

LavaCake::vec3f operator- (LavaCake::vec3f const& left,
  float const& right);

LavaCake::vec3f operator* (float           left,
  LavaCake::vec3f const& right);

LavaCake::vec3f operator* (LavaCake::vec3f const& left,
  float           right);

LavaCake::vec3f operator* (LavaCake::vec3f const& left,
  LavaCake::mat4 const& right);

LavaCake::vec3f operator- (LavaCake::vec3f const& vector);

bool operator== (LavaCake::vec3f const& left,
  LavaCake::vec3f const& right);

LavaCake::mat4 operator* (LavaCake::mat4 const& left,
  LavaCake::mat4 const& right);

#endif // TOOLS