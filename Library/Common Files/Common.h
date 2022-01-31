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
// � Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329
//
// Common

#ifndef COMMON
#define COMMON



#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <thread>
#include <cmath>
#include <functional>
#include <memory>
#include "VulkanFunctions.h"

#define GLFW_INCLUDE_NONE
#ifdef _WIN32
    #include <Windows.h>
    #define GLFW_EXPOSE_NATIVE_WIN32 true
#elif __APPLE__
    #define GLFW_INCLUDE_VULKAN
#endif

#include "glfw3.h"
#include "glfw3native.h"

namespace LavaCake {

  // Vulkan library type
#ifdef _WIN32
#define LIBRARY_TYPE HMODULE
#elif defined __linux
#define LIBRARY_TYPE void*
#elif defined __APPLE__
#define LIBRARY_TYPE void*
#endif

 
  struct WindowParameters {

    GLFWwindow*     Window;
    
      
  };

  // Extension availability check
  bool IsExtensionSupported( std::vector<VkExtensionProperties> const & available_extensions,
                             char const * const                         extension );

} // namespace LavaCake

#endif // COMMON
