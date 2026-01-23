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
- GLFW3 (for windowed applications)

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

## Quick Start

### Clear Screen Example

```cpp
#include <LavaCake/Device.hpp>
#include <LavaCake/SwapChainImage.hpp>

int main() {
    // Create device with window
    LavaCake::Device device;
    device.initDevice(800, 600, "LavaCake Window");
    device.initSwapChain();

    while (!glfwWindowShouldClose(device.getWindow())) {
        glfwPollEvents();

        // Acquire swapchain image
        auto [swapchainImage, swapchainIndex] = device.acquireSwapChainImage();
        auto& commandBuffer = device.getCommandBuffer();

        commandBuffer.wait();
        commandBuffer.resetFence();
        commandBuffer.begin();

        // Transition and clear
        swapchainImage.transitionImageLayout(
            commandBuffer.getHandle(),
            vk::ImageLayout::eTransferDstOptimal
        );

        vk::ClearColorValue clearColor(std::array<float, 4>{0.2f, 0.3f, 0.4f, 1.0f});
        commandBuffer.getHandle().clearColorImage(
            swapchainImage.getImage(),
            vk::ImageLayout::eTransferDstOptimal,
            clearColor,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        swapchainImage.prepareForPresent(commandBuffer.getHandle());
        commandBuffer.end();
        commandBuffer.submit(device.getGraphicQueue());

        device.presentImage(swapchainIndex);
    }

    device.waitIdle();
    return 0;
}
```

### Compute Shader Example

```cpp
#include <LavaCake/Device.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/DescriptorSet.hpp>

int main() {
    // Create headless device (no window)
    LavaCake::Device device;
    device.initDevice();

    // Create buffers
    std::vector<float> inputA = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> inputB = {5.0f, 6.0f, 7.0f, 8.0f};

    LavaCake::Buffer bufferA(device, inputA, vk::BufferUsageFlagBits::eStorageBuffer);
    LavaCake::Buffer bufferB(device, inputB, vk::BufferUsageFlagBits::eStorageBuffer);
    LavaCake::Buffer bufferC(device, 4 * sizeof(float),
                             vk::BufferUsageFlagBits::eStorageBuffer,
                             VMA_MEMORY_USAGE_GPU_TO_CPU);

    // Create compute pipeline
    LavaCake::ComputePipeline pipeline(device);
    pipeline.setComputeShader("shaders/vector_addition.comp");
    pipeline.addStorageBuffer(0, 0);
    pipeline.addStorageBuffer(0, 1);
    pipeline.addStorageBuffer(0, 2);
    pipeline.compile();

    // Bind and dispatch
    // ... (see examples for full code)

    return 0;
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

### Compute
| Example | Description |
|---------|-------------|
| `01_vector_addition` | Headless compute with storage buffers |

Build examples with:
```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .
```

## Project Structure

```
LavaCake/
├── Library/LavaCake/     # Header-only library
│   ├── Device.hpp        # Core device management
│   ├── Buffer.hpp        # GPU buffer handling
│   ├── Image.hpp         # Image and sampler management
│   ├── CommandBuffer.hpp # Command recording
│   ├── GraphicPipeline.hpp
│   ├── ComputePipeline.hpp
│   ├── DescriptorSet.hpp
│   ├── ShaderModule.hpp
│   └── DynamicRendering.hpp
├── examples/             # Usage examples
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
