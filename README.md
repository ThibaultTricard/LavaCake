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
- **ImGui Integration** - Built-in ImGuiRenderer for easy UI overlay support
- **Swapchain Resizing** - Simple `resizeSwapchain()` with automatic teardown and recreation
- **Cross-Platform** - Works on macOS, Linux, and Windows (Windows support is untested)

## Requirements

- C++20 compatible compiler
- CMake 3.10+
- Vulkan SDK 1.3+
- A windowing library (GLFW3, SDL2, Qt, etc.) - only for graphics applications

## Installation

### Using CMake FetchContent (Recommended)

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

### Using an Installed Version

If you've installed LavaCake to your system (see [Building from Source](#building-from-source)):

```cmake
find_package(LavaCake REQUIRED)
target_link_libraries(your_target PRIVATE LavaCake::LavaCake)
```

If installed to a non-standard location, specify the prefix:
```bash
cmake -DCMAKE_PREFIX_PATH=/your/install/path ..
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

Common install prefixes:
- **macOS (Apple Silicon)**: `/opt/homebrew`
- **macOS (Intel)**: `/usr/local`
- **Linux**: `/usr/local` or `/usr`

The installation includes:
- LavaCake headers
- VulkanMemoryAllocator header (`vk_mem_alloc.h`)
- shaderc headers and library (`libshaderc_combined.a`)
- CMake config files for `find_package(LavaCake)`

To build without examples (no GLFW dependency required):
```bash
cmake .. -DLAVACAKE_BUILD_EXAMPLES=OFF
```

### Uninstalling

After installation, CMake creates `build/install_manifest.txt` listing all installed files. To uninstall:

```bash
xargs rm -f < build/install_manifest.txt
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

    // Create device with GLFW window using factory function
    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    auto device = LavaCake::createWindowedDevice(surfaceConfig, 1);  // 1 graphics queue

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

### With SDL2

```cpp
#include <LavaCake/SDL2Support.hpp>  // Convenience header for SDL2 users
#include <LavaCake/CommandBuffer.hpp>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("LavaCake",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_VULKAN);

    auto surfaceConfig = LavaCake::SDL2::createSurfaceConfig(window);
    auto device = LavaCake::createWindowedDevice(surfaceConfig, 1);  // 1 graphics queue
    // ...
}
```

### With Other Windowing Libraries (Qt, etc.)

LavaCake is window-manager agnostic. Provide your own `SurfaceConfig`:

```cpp
#include <LavaCake/Device.hpp>

int main() {
    // Create your window using your preferred library...

    LavaCake::SurfaceConfig config;

    // Get required Vulkan extensions from your windowing library
    config.requiredExtensions = { /* VK_KHR_surface, platform-specific extensions */ };

    // Provide surface creation callback
    config.createSurface = [](vk::Instance instance) -> vk::SurfaceKHR {
        VkSurfaceKHR surface;
        // Create surface using your windowing library's API
        return vk::SurfaceKHR(surface);
    };

    auto device = LavaCake::createWindowedDevice(config, 1);
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
    auto device = LavaCake::createHeadlessDevice(1, 1);  // 1 graphics queue, 1 compute queue

    // See examples/compute/01_vector_addition.cpp for complete code
}
```

## Device Creation

LavaCake provides multiple ways to create a Vulkan device, from simple factory functions to a full-featured builder pattern.

### Factory Functions (Recommended for Most Use Cases)

Quick device creation with sensible defaults:

```cpp
// Basic headless device - 1 graphics queue, validation enabled
auto device = LavaCake::createBasicDevice();

// Windowed device with surface - custom queue counts
auto device = LavaCake::createWindowedDevice(surfaceConfig, 2, 1);  // 2 graphics, 1 compute

// Headless device - for compute or offscreen rendering
auto device = LavaCake::createHeadlessDevice(1, 0);  // 1 graphics, 0 compute

// Advanced device - includes anisotropic filtering and descriptor indexing
auto device = LavaCake::createAdvancedDevice(surfaceConfig, 2, 1);
```

### Builder Pattern (For Advanced Configuration)

Use the builder for full control over device features:

```cpp
// Custom device with specific features
auto device = LavaCake::createDeviceBuilder()
    .setGraphicQueueCount(2)
    .setComputeQueueCount(1)
    .setSurface(surfaceConfig)
    .enableSamplerAnisotropy(true)
    .enableGeometryShader(true)
    .setApplicationInfo("MyApp", VK_MAKE_VERSION(1, 0, 0))
    .setApiVersion(VK_API_VERSION_1_3)
    .preferDiscreteGPU()
    .build();

// Compute-only device with custom GPU selection
auto device = LavaCake::createDeviceBuilder()
    .setGraphicQueueCount(0)
    .setComputeQueueCount(4)
    .headless()
    .setDeviceSelector([](vk::PhysicalDevice dev) {
        // Custom GPU scoring logic
        auto props = dev.getProperties();
        return (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) ? 100000 : 1000;
    })
    .build();

// Advanced features with Vulkan 1.2+ feature structures
vk::PhysicalDeviceVulkan12Features vulkan12{};
vulkan12.bufferDeviceAddress = VK_TRUE;
vulkan12.descriptorIndexing = VK_TRUE;

auto device = LavaCake::createDeviceBuilder()
    .setGraphicQueueCount(1)
    .addFeature(vulkan12)  // Generic method - works with ANY Vulkan feature struct
    .headless()
    .build();
```

### Key Features

- **Dynamic Rendering**: Always enabled (mandatory) - no need for explicit render passes
- **Fluent API**: Method chaining for readable configuration
- **Generic Feature Chain**: Support for any Vulkan feature structure via `addFeature<T>()`
- **Automatic Extensions**: Swapchain extensions added automatically when surface is configured
- **VMA Integration**: Vulkan Memory Allocator automatically initialized with customizable flags
- **Validation Layers**: Enabled by default, can be disabled for release builds

## Swapchain Resizing

When the window is resized the swapchain must be recreated to match the new surface extent. `device.resizeSwapchain()` handles teardown and recreation automatically — it waits for the GPU to idle, destroys the old swapchain images, and rebuilds everything from the surface's current extent.

Two things can trigger a resize:

1. The window manager fires a resize callback **before** the next frame starts — detected via a flag.
2. `acquireNextImageKHR` or `presentKHR` returns `OutOfDateKHR` on a frame that was **already in flight** when the resize happened — this surfaces as a `vk::OutOfDateKHRError` exception from vulkan.hpp.

Both paths need to be handled. Here is the complete GLFW pattern:

```cpp
// --- setup (once, after window creation) ---
bool framebufferResized = false;
glfwSetWindowUserPointer(window, &framebufferResized);
glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int, int) {
    *static_cast<bool*>(glfwGetWindowUserPointer(win)) = true;
});

// --- render loop ---
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Path 1: resize callback fired before this frame
    if (framebufferResized) {
        framebufferResized = false;
        // Spin while minimized — zero extent is invalid for swapchain creation
        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        while (w == 0 || h == 0) {
            glfwWaitEvents();
            glfwGetFramebufferSize(window, &w, &h);
        }
        device.resizeSwapchain();
        continue;
    }

    // Path 2: OutOfDateKHR on an in-flight frame
    try {
        auto& image = device.aquireSwapChainImage(imageAvailableSemaphore);
        // ... record commands, submit, present ...
        device.presentImage(image, {renderFinishedSemaphore});
    } catch (const vk::OutOfDateKHRError&) {
        framebufferResized = true;  // will be handled at the top of the next iteration
        continue;
    }
}
```

Key points:
- `resizeSwapchain()` calls `waitIdle` internally, so all in-flight GPU work completes before the old swapchain is destroyed.
- The minimization spin (`while (w == 0 || h == 0)`) prevents attempting to create a swapchain with a zero extent.
- The `OutOfDateKHR` catch is necessary because the resize callback and the present call are not atomic — a resize can arrive between the flag check and the end of the frame.
- See `examples/basic/02_colored_triangle.cpp` for a complete working example.

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
| `09_rotating_cube` | 3D cube with depth testing and MVP transformations |

### Advanced
| Example | Description |
|---------|-------------|
| `01_shadow_map` | Two-pass shadow mapping with PCF soft shadows |
| `02_multiple_lights` | Multiple point lights with bindless shadow map arrays |

### Compute
| Example | Description |
|---------|-------------|
| `01_vector_addition` | Headless compute with storage buffers |

### ImGui
| Example | Description |
|---------|-------------|
| `01_imgui_demo` | ImGui integration with LavaCake's ImGuiRenderer |

Build examples with (requires GLFW):
```bash
cd build
cmake .. -DLAVACAKE_BUILD_EXAMPLES=ON
cmake --build .
```

## Project Structure

```
LavaCake/
├── Library/LavaCake/       # Header-only library
│   ├── Device.hpp          # Core device management + SurfaceConfig
│   ├── GLFWSupport.hpp     # GLFW surface configuration
│   ├── SDL2Support.hpp     # SDL2 surface configuration
│   ├── Buffer.hpp          # GPU buffer handling
│   ├── UniformBuffer.hpp   # Uniform buffer utilities
│   ├── Image.hpp           # Image and sampler management
│   ├── SwapChainImage.hpp  # Swapchain image handling
│   ├── CommandBuffer.hpp   # Command recording
│   ├── Pipeline.hpp        # Base pipeline class
│   ├── GraphicPipeline.hpp # Graphics pipeline
│   ├── ComputePipeline.hpp # Compute pipeline
│   ├── DescriptorSet.hpp   # Descriptor set management
│   ├── DescriptorPool.hpp  # Descriptor pool management
│   ├── ShaderModule.hpp    # Shader compilation
│   ├── DynamicRendering.hpp# Dynamic rendering utilities
│   ├── ImGui.hpp           # ImGui integration
│   ├── VMAFlags.hpp        # VMA configuration flags
│   └── ByteDictionary.hpp  # Byte buffer utilities
├── examples/               # Usage examples
│   ├── basic/              # Introductory examples
│   ├── advanced/           # Shadow mapping, multiple lights
│   ├── compute/            # GPGPU examples
│   └── imgui/              # ImGui integration
├── cmake/                  # CMake configuration
└── documentation/          # Doxygen config
```

## API Design

LavaCake uses modern C++ patterns throughout:

- **Builder Pattern** - Fluent API for device, pipeline, and descriptor configuration
  - Factory functions (`createBasicDevice`, `createWindowedDevice`, etc.) for common use cases
  - Full builder API (`Device::Builder`) for advanced customization
  - Generic feature chain support via templates
- **RAII** - Automatic resource cleanup when objects go out of scope
- **Move Semantics** - Efficient resource transfer without copying
- **Header-Only** - All implementations inline for easy integration
- **Type Safety** - Leverages Vulkan-HPP for type-safe Vulkan API usage

## License

MIT License - see [LICENCE](LICENCE) for details.

## Author

Thibault Tricard
