/**
 * LavaCake Example: Cornell Box Ray Tracing
 *
 * This example demonstrates:
 * - Creating a ray tracing device with RTX support
 * - Building acceleration structures (BLAS and TLAS)
 * - Creating a ray tracing pipeline with raygen, miss, and closest-hit shaders
 * - Creating a shader binding table (SBT)
 * - Rendering the classic Cornell Box scene with ray tracing
 *
 * The Cornell Box is rendered with proper lighting and displayed to a window.
 */

#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/AccelerationStructure.hpp>
#include <LavaCake/RayTracingPipeline.hpp>
#include <LavaCake/ShaderBindingTable.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/Image.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/UniformBuffer.hpp>
#include <iostream>
#include <array>
#include <cstring>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// Vertex structure matching the shader
struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
};

// Helper function to add a quad (two triangles) to the geometry
void addQuad(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
             const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
             const glm::vec3& normal, const glm::vec3& color) {
    uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

    vertices.push_back({v0, normal, color});
    vertices.push_back({v1, normal, color});
    vertices.push_back({v2, normal, color});
    vertices.push_back({v3, normal, color});

    // First triangle
    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);

    // Second triangle
    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

// Helper to add a box
void addBox(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
            const glm::vec3& minCorner, const glm::vec3& maxCorner, const glm::vec3& color) {
    glm::vec3 v0 = minCorner;
    glm::vec3 v1 = glm::vec3(maxCorner.x, minCorner.y, minCorner.z);
    glm::vec3 v2 = glm::vec3(maxCorner.x, minCorner.y, maxCorner.z);
    glm::vec3 v3 = glm::vec3(minCorner.x, minCorner.y, maxCorner.z);
    glm::vec3 v4 = glm::vec3(minCorner.x, maxCorner.y, minCorner.z);
    glm::vec3 v5 = glm::vec3(maxCorner.x, maxCorner.y, minCorner.z);
    glm::vec3 v6 = maxCorner;
    glm::vec3 v7 = glm::vec3(minCorner.x, maxCorner.y, maxCorner.z);

    // Bottom face
    addQuad(vertices, indices, v0, v3, v2, v1, glm::vec3(0, -1, 0), color);
    // Top face
    addQuad(vertices, indices, v4, v5, v6, v7, glm::vec3(0, 1, 0), color);
    // Front face
    addQuad(vertices, indices, v3, v7, v6, v2, glm::vec3(0, 0, 1), color);
    // Back face
    addQuad(vertices, indices, v0, v1, v5, v4, glm::vec3(0, 0, -1), color);
    // Left face
    addQuad(vertices, indices, v0, v4, v7, v3, glm::vec3(-1, 0, 0), color);
    // Right face
    addQuad(vertices, indices, v1, v2, v6, v5, glm::vec3(1, 0, 0), color);
}

// Create Cornell Box geometry
void createCornellBox(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    // Colors
    glm::vec3 white(0.73f, 0.73f, 0.73f);
    glm::vec3 red(0.65f, 0.05f, 0.05f);
    glm::vec3 green(0.12f, 0.45f, 0.15f);

    // Room dimensions: -1 to 1 in X and Z, 0 to 2 in Y
    float roomSize = 1.0f;

    // Floor (white)
    addQuad(vertices, indices,
            glm::vec3(-roomSize, 0, -roomSize),
            glm::vec3(-roomSize, 0, roomSize),
            glm::vec3(roomSize, 0, roomSize),
            glm::vec3(roomSize, 0, -roomSize),
            glm::vec3(0, 1, 0), white);

    // Ceiling (white)
    addQuad(vertices, indices,
            glm::vec3(-roomSize, 2 * roomSize, -roomSize),
            glm::vec3(roomSize, 2 * roomSize, -roomSize),
            glm::vec3(roomSize, 2 * roomSize, roomSize),
            glm::vec3(-roomSize, 2 * roomSize, roomSize),
            glm::vec3(0, -1, 0), white);

    // Back wall (white)
    addQuad(vertices, indices,
            glm::vec3(-roomSize, 0, -roomSize),
            glm::vec3(roomSize, 0, -roomSize),
            glm::vec3(roomSize, 2 * roomSize, -roomSize),
            glm::vec3(-roomSize, 2 * roomSize, -roomSize),
            glm::vec3(0, 0, 1), white);

    // Left wall (red)
    addQuad(vertices, indices,
            glm::vec3(-roomSize, 0, roomSize),
            glm::vec3(-roomSize, 0, -roomSize),
            glm::vec3(-roomSize, 2 * roomSize, -roomSize),
            glm::vec3(-roomSize, 2 * roomSize, roomSize),
            glm::vec3(1, 0, 0), red);

    // Right wall (green)
    addQuad(vertices, indices,
            glm::vec3(roomSize, 0, -roomSize),
            glm::vec3(roomSize, 0, roomSize),
            glm::vec3(roomSize, 2 * roomSize, roomSize),
            glm::vec3(roomSize, 2 * roomSize, -roomSize),
            glm::vec3(-1, 0, 0), green);

    // Tall box (white) - rotated slightly
    addBox(vertices, indices,
           glm::vec3(-0.6f, 0.0f, -0.6f),
           glm::vec3(-0.1f, 1.2f, 0.0f),
           white);

    // Short box (white)
    addBox(vertices, indices,
           glm::vec3(0.1f, 0.0f, 0.0f),
           glm::vec3(0.6f, 0.6f, 0.5f),
           white);
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Cornell Box - Ray Tracing Example", nullptr, nullptr);

    // Create ray tracing device
    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createRayTracingDevice(surfaceConfig, 1);

    std::cout << "Ray tracing device created successfully!\n";

    { // Context scope for GPU objects

        // Get RT properties
        auto rtProps = device.getRayTracingPipelineProperties();
        std::cout << "Shader group handle size: " << rtProps.shaderGroupHandleSize << "\n";
        std::cout << "Max recursion depth: " << rtProps.maxRayRecursionDepth << "\n";

        // Create Cornell Box geometry
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        createCornellBox(vertices, indices);

        std::cout << "Cornell Box geometry created: " << vertices.size() << " vertices, "
                  << indices.size() / 3 << " triangles\n";

        // Create vertex and index buffers for AS build
        LavaCake::Buffer vertexBuffer(device, vertices,
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
            vk::BufferUsageFlagBits::eShaderDeviceAddress |
            vk::BufferUsageFlagBits::eStorageBuffer);

        LavaCake::Buffer indexBuffer(device, indices,
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
            vk::BufferUsageFlagBits::eShaderDeviceAddress |
            vk::BufferUsageFlagBits::eStorageBuffer);

        // Get buffer device addresses
        vk::BufferDeviceAddressInfo vertexAddressInfo{};
        vertexAddressInfo.buffer = vertexBuffer;
        vk::DeviceAddress vertexBufferAddress = device.getDevice().getBufferAddress(vertexAddressInfo);

        vk::BufferDeviceAddressInfo indexAddressInfo{};
        indexAddressInfo.buffer = indexBuffer;
        vk::DeviceAddress indexBufferAddress = device.getDevice().getBufferAddress(indexAddressInfo);

        // Build BLAS
        std::cout << "Building BLAS...\n";
        LavaCake::BottomLevelAccelerationStructure blas =
            LavaCake::BottomLevelAccelerationStructure::Builder(device)
                .addTriangleGeometry()
                .setVertexBuffer(vertexBufferAddress, sizeof(Vertex), static_cast<uint32_t>(vertices.size()))
                .setIndexBuffer(indexBufferAddress, static_cast<uint32_t>(indices.size()))
                .setPreferFastTrace(true)
                .build();
        std::cout << "BLAS built successfully!\n";

        // Create instance for TLAS
        vk::TransformMatrixKHR transformMatrix = {
            std::array<std::array<float, 4>, 3>{{
                {1.0f, 0.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f, 0.0f}
            }}
        };

        vk::AccelerationStructureInstanceKHR instance{};
        instance.transform = transformMatrix;
        instance.instanceCustomIndex = 0;
        instance.mask = 0xFF;
        instance.instanceShaderBindingTableRecordOffset = 0;
        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instance.accelerationStructureReference = blas.getDeviceAddress();

        // Create instance buffer
        LavaCake::Buffer instanceBuffer(device,
            std::vector<vk::AccelerationStructureInstanceKHR>{instance},
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
            vk::BufferUsageFlagBits::eShaderDeviceAddress);

        vk::BufferDeviceAddressInfo instanceAddressInfo{};
        instanceAddressInfo.buffer = instanceBuffer;
        vk::DeviceAddress instanceBufferAddress = device.getDevice().getBufferAddress(instanceAddressInfo);

        // Build TLAS
        std::cout << "Building TLAS...\n";
        LavaCake::TopLevelAccelerationStructure tlas =
            LavaCake::TopLevelAccelerationStructure::Builder(device)
                .setInstances(instanceBufferAddress, 1)
                .setPreferFastTrace(true)
                .build();
        std::cout << "TLAS built successfully!\n";

        // Create storage image for ray tracing output
        vk::Extent2D extent = device.getSwapchainExtent();
        std::cout << "Swapchain extent: " << extent.width << "x" << extent.height << "\n";
        LavaCake::Image storageImage(device, extent.width, extent.height, 1,
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eSampled);

        LavaCake::ImageView storageImageView(storageImage, vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eColor);

        // Create accumulation image (RGBA32F for high precision accumulation)
        LavaCake::Image accumulationImage(device, extent.width, extent.height, 1,
            vk::Format::eR32G32B32A32Sfloat,
            vk::ImageUsageFlagBits::eStorage);

        LavaCake::ImageView accumulationImageView(accumulationImage, vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eColor);

        // Transition images to general layout
        {
            LavaCake::CommandBuffer cmd(device);
            cmd.begin();
            storageImage.transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
            accumulationImage.transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
            cmd.end();
            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = cmd;
            device.getGraphicQueue(0).submit(submitInfo);
            device.getGraphicQueue(0).waitIdle();
        }

        // Create camera buffer
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 1.0f, 3.5f),  // Camera position
            glm::vec3(0.0f, 1.0f, 0.0f),  // Look at center
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
        );
        glm::mat4 proj = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(extent.width) / static_cast<float>(extent.height),
            0.1f, 100.0f
        );
        proj[1][1] *= -1;  // Flip Y for Vulkan

        LavaCake::UniformBuffer cameraBuffer(device);
        cameraBuffer.addVariable("viewInverse", glm::inverse(view));
        cameraBuffer.addVariable("projInverse", glm::inverse(proj));
        cameraBuffer.addVariable("frameIndex", 0u);
        cameraBuffer.addVariable("padding", std::array<uint32_t, 3>{0, 0, 0});
        cameraBuffer.end();

        uint32_t frameIndex = 0;

        // Create descriptor set layout for ray tracing
        LavaCake::DescriptorSetLayout rtDescriptorLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addAccelerationStructure(0, vk::ShaderStageFlagBits::eRaygenKHR)  // TLAS
                .addStorageImage(1, vk::ShaderStageFlagBits::eRaygenKHR)            // Output image
                .addUniformBuffer(2, vk::ShaderStageFlagBits::eRaygenKHR)           // Camera
                .addStorageBuffer(3, vk::ShaderStageFlagBits::eClosestHitKHR)       // Vertices
                .addStorageBuffer(4, vk::ShaderStageFlagBits::eClosestHitKHR)       // Indices
                .addStorageImage(5, vk::ShaderStageFlagBits::eRaygenKHR)            // Accumulation image
                .build();

        // Create descriptor pool
        LavaCake::DescriptorPool rtDescriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addAccelerationStructures(1)
                .addStorageImages(2)  // Output + Accumulation
                .addUniformBuffers(1)
                .addStorageBuffers(2)
                .setMaxSets(1)
                .build();

        // Allocate and update descriptor set
        vk::DescriptorSet rtDescriptorSet = rtDescriptorPool.allocate(rtDescriptorLayout);

        // Get vk::ImageView for storage image bindings
        vk::ImageView storageVkImageView = storageImageView.getImageView();
        vk::ImageView accumulationVkImageView = accumulationImageView.getImageView();

        LavaCake::DescriptorSetUpdater(device, rtDescriptorSet)
            .bindAccelerationStructure(0, tlas)
            .bindStorageImage(1, storageVkImageView, vk::ImageLayout::eGeneral)
            .bindUniformBuffer(2, cameraBuffer, 0, cameraBuffer.size())
            .bindStorageBuffer(3, vertexBuffer, 0, vertices.size() * sizeof(Vertex))
            .bindStorageBuffer(4, indexBuffer, 0, indices.size() * sizeof(uint32_t))
            .bindStorageImage(5, accumulationVkImageView, vk::ImageLayout::eGeneral)
            .update();

        std::cout << "Descriptor set updated\n";

        // Create ray tracing pipeline
        std::cout << "Creating ray tracing pipeline...\n";
        LavaCake::RayTracingPipeline rtPipeline =
            LavaCake::RayTracingPipeline::Builder(device)
                .addRaygenShaderFromFile(root + "raytracing/shaders/raygen.rgen", LavaCake::ShadingLanguage::eGLSL)
                .addMissShaderFromFile(root + "raytracing/shaders/miss.rmiss", LavaCake::ShadingLanguage::eGLSL)
                .addMissShaderFromFile(root + "raytracing/shaders/shadow.rmiss", LavaCake::ShadingLanguage::eGLSL)
                .beginHitGroup()
                    .setClosestHitShaderFromFile(root + "raytracing/shaders/closesthit.rchit", LavaCake::ShadingLanguage::eGLSL)
                .endHitGroup()
                .setMaxRecursionDepth(31)
                .addDescriptorSetLayout(rtDescriptorLayout)
                .build();
        std::cout << "Ray tracing pipeline created!\n";

        // Create shader binding table
        std::cout << "Creating shader binding table...\n";
        LavaCake::ShaderBindingTable sbt(device, rtPipeline);
        std::cout << "SBT created!\n";

        // Create a simple fullscreen quad pipeline to display the result
        LavaCake::DescriptorSetLayout displayLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addCombinedImageSampler(0, vk::ShaderStageFlagBits::eFragment)
                .build();

        LavaCake::DescriptorPool displayPool =
            LavaCake::DescriptorPool::Builder(device)
                .addCombinedImageSamplers(1)
                .setMaxSets(1)
                .build();

        vk::DescriptorSet displayDescriptorSet = displayPool.allocate(displayLayout);

        // Create sampler
        LavaCake::Sampler sampler(device,
            vk::SamplerMipmapMode::eLinear,
            0.0f, VK_LOD_CLAMP_NONE, 0.0f,
            false, 1.0f,
            vk::Filter::eLinear, vk::Filter::eLinear,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge);

        LavaCake::DescriptorSetUpdater(device, displayDescriptorSet)
            .bindImage(0, storageImageView, sampler, vk::ImageLayout::eGeneral)
            .update();

        // Create display pipeline
        LavaCake::GraphicsPipeline displayPipeline =
            LavaCake::GraphicsPipeline::Builder(device)
                .addShaderFromFile(root + "raytracing/shaders/fullscreen.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
                .addShaderFromFile(root + "raytracing/shaders/fullscreen.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
                .addColorAttachmentFormat(device.getSwapchainFormat())
                .setBindlessVertexInput()
                .addDescriptorSetLayout(displayLayout)
                //.setCullMode(vk::CullModeFlagBits::eNone)
                .build();

        // Create semaphores
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);

        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore({});
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore({});
        }

        LavaCake::CommandBuffer cmdBuffer(device, true);
        uint32_t currentFrame = 0;

        std::cout << "Starting render loop...\n";

        // Main render loop
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // Wait for previous frame
            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            // Update camera buffer with current frame index
            cameraBuffer.setVariable("frameIndex", frameIndex);
            frameIndex++;

            // Acquire swapchain image
            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(
                imageAvailableSemaphores[currentFrame]);

            // Begin recording
            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // Update uniform buffer on GPU
            cameraBuffer.update(cmdBuffer.getCommandBuffer());

            // === Ray tracing pass ===
            rtPipeline.bind(cmdBuffer);
            cmdBuffer.getCommandBuffer().bindDescriptorSets(
                vk::PipelineBindPoint::eRayTracingKHR,
                rtPipeline.getLayout(),
                0, {rtDescriptorSet}, {});

            rtPipeline.traceRays(cmdBuffer,
                sbt.getRaygenRegion(),
                sbt.getMissRegion(),
                sbt.getHitRegion(),
                sbt.getCallableRegion(),
                extent.width, extent.height, 1);

            // Memory barrier to ensure ray tracing completes before display
            vk::MemoryBarrier memoryBarrier{};
            memoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
            memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            cmdBuffer.getCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eRayTracingShaderKHR,
                vk::PipelineStageFlagBits::eFragmentShader,
                {}, memoryBarrier, {}, {});

            // === Display pass ===
            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            LavaCake::DynamicRenderingContext renderContext =
                LavaCake::DynamicRenderingContext::Builder()
                    .setRenderArea(extent)
                    .addColorAttachment(swapchainImage,
                        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}))
                    .begin(cmdBuffer);

            renderContext.setDefaultViewportScissor(cmdBuffer);
            displayPipeline.bind(cmdBuffer);
            cmdBuffer.getCommandBuffer().bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                displayPipeline.getLayout(),
                0, {displayDescriptorSet}, {});
            displayPipeline.draw(cmdBuffer, 3);  // Fullscreen triangle

            renderContext.end(cmdBuffer);
            swapchainImage.prepareForPresentBarrier(cmdBuffer);

            cmdBuffer.end();

            // Submit
            vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submitInfo{};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
            submitInfo.pWaitDstStageMask = &waitStage;
            submitInfo.commandBufferCount = 1;
            vk::CommandBuffer rawCmd = cmdBuffer.getCommandBuffer();
            submitInfo.pCommandBuffers = &rawCmd;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

            device.getGraphicQueue(0).submit(submitInfo, cmdBuffer.getFence());
            cmdBuffer.markSubmitted();

            // Present
            device.presentImage(swapchainImage, {renderFinishedSemaphores[currentFrame]});
            currentFrame = (currentFrame + 1) % swapchainImageCount;
        }

        device.waitForAllCommands();

        // Cleanup
        for (size_t i = 0; i < swapchainImageCount; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }

    } // GPU objects destroyed

    device.releaseDevice();
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Cornell Box ray tracing example completed!\n";
    return 0;
}
