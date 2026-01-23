# LavaCake

A modern, header-only C++ wrapper library for Vulkan that simplifies GPU programming for both graphics and compute workloads.

## Overview

LavaCake provides high-level abstractions over the Vulkan API, letting you leverage powerful GPU capabilities without dealing with low-level boilerplate. Built on Vulkan-HPP and designed with modern C++20 features, it offers an intuitive API through builder patterns and RAII principles.

## Features

- **Simplified Device Setup** - Automatic GPU selection, instance creation, and queue management
- **Dynamic Rendering** - Modern Vulkan 1.3 rendering without explicit render passes
- **Compute Pipelines** - Streamlined compute shader support for GPGPU workloads
- **Integrated Memory Management** - Built-in VMA (Vulkan Memory Allocator) integration
- **Runtime Shader Compilation** - Automatic GLSL to SPIR-V compilation via shaderc
- **Bindless Rendering** - Support for modern bindless descriptor patterns
- **Cross-Platform** - Works on macOS, Linux, and Windows (Windows support is untested)

## Requirements

- C++20 compatible compiler
- CMake 3.10+
- Vulkan SDK 1.3+
- A windowing library (GLFW3, SDL2, Qt, etc.) - only for graphics applications

## Installation

### Using CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    LavaCake
    GIT_REPOSITORY https://github.com/ThibaultTricard/LavaCake.git
    GIT_TAG dev-2.0
)
FetchContent_MakeAvailable(LavaCake)

target_link_libraries(your_target PRIVATE LavaCake::LavaCake)
```

### Building from Source

```bash
git clone https://github.com/ThibaultTricard/LavaCake.git
cd LavaCake
mkdir build && cd build
cmake ..
cmake --build .
cmake --install . --prefix /your/install/path
```

To build without examples (no GLFW dependency required):
```bash
cmake .. -DLAVACAKE_BUILD_EXAMPLES=OFF
```

## Quick Start

### With GLFW

```cpp
#include <LavaCake/GLFWSupport.hpp>  // Convenience header for GLFW users
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/DynamicRendering.hpp>

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "LavaCake", nullptr, nullptr);

    // Create device with GLFW window
    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device(surfaceConfig, 1);  // 1 graphics queue

    LavaCake::CommandBuffer cmdBuffer(device, true);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Render frame...
        // See examples/basic/01_clear_screen.cpp for complete code
    }

    device.waitForAllCommands();
    device.releaseDevice();
    return 0;
}
```

### With Other Windowing Libraries (SDL2, Qt, etc.)

LavaCake is window-manager agnostic. Provide your own `SurfaceConfig`:

```cpp
#include <LavaCake/Device.hpp>
#include <SDL2/SDL_vulkan.h>

int main() {
    SDL_Window* window = SDL_CreateWindow("LavaCake",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_VULKAN);

    // Create surface config for SDL2
    LavaCake::SurfaceConfig config;

    // Get required extensions
    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    config.requiredExtensions.resize(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, config.requiredExtensions.data());

    // Provide surface creation callback
    config.createSurface = [window](vk::Instance instance) -> vk::SurfaceKHR {
        VkSurfaceKHR surface;
        SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), &surface);
        return vk::SurfaceKHR(surface);
    };

    LavaCake::Device device(config, 1);
    // ...
}
```

### Headless Compute

```cpp
#include <LavaCake/Device.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/ComputePipeline.hpp>

int main() {
    // Create headless device (no window, no surface)
    LavaCake::Device device(0, 1);  // 0 graphics queues, 1 compute queue

    // See examples/compute/01_vector_addition.cpp for complete code
}
```

## Examples

The `examples/` directory contains progressive tutorials:

### Basic Graphics
| Example | Description |
|---------|-------------|
| `01_clear_screen` | Minimal setup with swapchain and color clear |
| `02_colored_triangle` | Graphics pipeline and shader compilation |
| `03_uniform_buffer` | Passing uniform data to shaders |
| `04_vertex_buffer` | Vertex buffer creation and usage |
| `05_indexed_quad` | Indexed drawing with index buffers |
| `06_bindless_quad` | Modern bindless rendering pattern |
| `07_bindless_quad_textured` | Bindless rendering with textures |
| `08_bindless_combined` | Bindless texture and buffer arrays |

### Compute
| Example | Description |
|---------|-------------|
| `01_vector_addition` | Headless compute with storage buffers |

Build examples with (requires GLFW):
```bash
cd build
cmake .. -DLAVACAKE_BUILD_EXAMPLES=ON
cmake --build .
```

## Project Structure

```
LavaCake/
├── Library/LavaCake/     # Header-only library
│   ├── Device.hpp        # Core device management + SurfaceConfig
│   ├── GLFWSupport.hpp   # Optional GLFW convenience utilities
│   ├── Buffer.hpp        # GPU buffer handling
│   ├── Image.hpp         # Image and sampler management
│   ├── CommandBuffer.hpp # Command recording
│   ├── GraphicPipeline.hpp
│   ├── ComputePipeline.hpp
│   ├── DescriptorSet.hpp
│   ├── ShaderModule.hpp
│   └── DynamicRendering.hpp
├── examples/             # Usage examples (requires GLFW)
├── cmake/                # CMake configuration
└── documentation/        # Doxygen config
```

## API Design

LavaCake uses modern C++ patterns throughout:

- **Builder Pattern** - Fluent API for pipeline and descriptor configuration
- **RAII** - Automatic resource cleanup when objects go out of scope
- **Move Semantics** - Efficient resource transfer without copying

## License

MIT License - see [LICENCE](LICENCE) for details.

## Author

Thibault Tricard
