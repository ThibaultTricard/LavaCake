/**
 * LavaCake Example 08: Bindless Combined (Texture & Buffer Arrays)
 *
 * This example demonstrates:
 * - Bindless texture arrays with dynamic indexing
 * - Bindless storage buffer arrays for per-object data
 * - Push constants to select texture/buffer indices at runtime
 * - Non-uniform resource indexing (GL_EXT_nonuniform_qualifier)
 * - Complete modern bindless rendering setup
 *
 * Four quads are rendered, each using a different texture and transform
 * selected dynamically via push constants.
 */

#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/Image.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>
#include <iostream>
#include <array>
#include <cmath>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// Push constant structure for selecting resources
struct PushConstants {
    uint32_t textureIndex;
    uint32_t transformIndex;
};

// Transform data structure stored in buffer array
struct TransformData {
    float offsetX;
    float offsetY;
    float scale;
    float padding; // Align to 16 bytes
};

int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "08 - Bindless Combined Example", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig,1);
    { // Create a context to make sure all GPU object are destroyed before we release the device 

        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Example 08: Bindless Combined (Texture & Buffer Arrays)\n";

        // ========== VERTEX DATA (same quad, will be transformed in shader) ==========

        // Vertex positions (vec2) - unit quad centered at origin
        std::vector<float> positions = {
            -0.2f, -0.2f,  // Bottom-left
            0.2f, -0.2f,  // Bottom-right
            0.2f,  0.2f,  // Top-right
            -0.2f,  0.2f   // Top-left
        };

        // Vertex UV (vec2)
        std::vector<float> UV = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
        };

        // Indices - 6 indices for 2 triangles
        std::vector<uint32_t> indices = {
            0, 1, 2,
            0, 2, 3
        };

        // Create storage buffers for vertex data
        LavaCake::Buffer positionBuffer(device, positions, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer UVBuffer(device, UV, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer indexBuffer(device, indices, vk::BufferUsageFlagBits::eStorageBuffer);

        std::cout << "Vertex storage buffers created\n";

        // ========== CREATE 4 DIFFERENT TEXTURES ==========

        const uint32_t imageSize = 64;
        const uint32_t cellSize = 8;

        // Helper to create a procedural texture with a specific color
        auto createCheckerTexture = [&](float r, float g, float b) {
            std::vector<float> imageData(imageSize * imageSize * 4);
            for(uint32_t i = 0; i < imageSize; i++) {
                for(uint32_t j = 0; j < imageSize; j++) {
                    float checker = ((i / cellSize) + (j / cellSize)) % 2 == 0 ? 1.0f : 0.3f;
                    uint32_t idx = (i + j * imageSize) * 4;
                    imageData[idx + 0] = r * checker;
                    imageData[idx + 1] = g * checker;
                    imageData[idx + 2] = b * checker;
                    imageData[idx + 3] = 1.0f;
                }
            }
            return imageData;
        };

        // Create 4 textures with different colors
        auto redData = createCheckerTexture(1.0f, 0.2f, 0.2f);
        auto greenData = createCheckerTexture(0.2f, 1.0f, 0.2f);
        auto blueData = createCheckerTexture(0.2f, 0.2f, 1.0f);
        auto yellowData = createCheckerTexture(1.0f, 1.0f, 0.2f);

        LavaCake::Image redTexture(device, redData, imageSize, imageSize, 1, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eSampled);
        LavaCake::Image greenTexture(device, greenData, imageSize, imageSize, 1, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eSampled);
        LavaCake::Image blueTexture(device, blueData, imageSize, imageSize, 1, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eSampled);
        LavaCake::Image yellowTexture(device, yellowData, imageSize, imageSize, 1, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eSampled);

        std::vector<LavaCake::ImageView> textureViews;
        textureViews.reserve(4);
        textureViews.emplace_back(redTexture);
        textureViews.emplace_back(greenTexture);
        textureViews.emplace_back(blueTexture);
        textureViews.emplace_back(yellowTexture);


        LavaCake::Sampler textureSampler(device);

        std::cout << "4 textures created (red, green, blue, yellow checkerboards)\n";

        // ========== CREATE 4 TRANSFORM BUFFERS ==========

        // Each transform positions the quad in a different quadrant
        std::vector<TransformData> transforms = {
            { -0.5f, -0.5f, 1.0f, 0.0f },  // Bottom-left
            {  0.5f, -0.5f, 1.0f, 0.0f },  // Bottom-right
            { -0.5f,  0.5f, 1.0f, 0.0f },  // Top-left
            {  0.5f,  0.5f, 1.0f, 0.0f }   // Top-right
        };

        LavaCake::Buffer transformBuffer(
            device,
            transforms,
            vk::BufferUsageFlagBits::eStorageBuffer
        );

        std::cout << "Transform buffer created with 4 transforms\n";

        // ========== CREATE BINDLESS DESCRIPTOR SET ==========

        const uint32_t MAX_TEXTURES = 16;  // Maximum textures in the bindless array

        // Create descriptor set layout with bindless texture array
        LavaCake::DescriptorSetLayout descriptorSetLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                // Vertex data (standard bindings)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)  // positions
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)  // UVs
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eVertex)  // indices
                // Transform buffer (single buffer containing array of transforms)
                .addStorageBuffer(3, vk::ShaderStageFlagBits::eVertex)
                // Texture array (bindless) - uses partial binding and update after bind
                .addCombinedImageSampler(4, vk::ShaderStageFlagBits::eFragment, MAX_TEXTURES)
                .setBindless(MAX_TEXTURES)
                .build();

        // Create descriptor pool with update-after-bind support
        LavaCake::DescriptorPool descriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(4)  // 3 vertex buffers + 1 transform buffer
                .addCombinedImageSamplers(MAX_TEXTURES)
                .setMaxSets(1)
                .setUpdateAfterBindFlag()
                .build();

        // Allocate descriptor set
        vk::DescriptorSet descriptorSet = descriptorPool.allocate(descriptorSetLayout);

        
        // Update descriptor set
        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, positionBuffer)
            .bindStorageBuffer(1, UVBuffer)
            .bindStorageBuffer(2, indexBuffer)
            .bindStorageBuffer(3, transformBuffer)
            .bindImageArray(4, textureViews, textureSampler)
            .update();

        std::cout << "Bindless descriptor set created and updated\n";

        // ========== CREATE GRAPHICS PIPELINE ==========

        LavaCake::GraphicsPipeline graphicPipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/bindless_combined.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/bindless_combined.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .addColorAttachmentFormat(device.getSwapchainFormat())
            .setBindlessVertexInput()
            .addDescriptorSetLayout(descriptorSetLayout)
            .addPushConstant<PushConstants>(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .build();

        std::cout << "Graphics pipeline created with push constants\n";

        // ========== SETUP SYNCHRONIZATION ==========

        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        vk::SemaphoreCreateInfo semaphoreInfo;
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);

        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }

        uint32_t currentFrame = 0;

        // ========== MAIN RENDER LOOP ==========

        std::cout << "\nRendering 4 quads with bindless textures and transforms...\n";

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            LavaCake::DynamicRenderingContext renderingContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage,
                    vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.15f, 1.0f})
                )
                .begin(cmdBuffer);

            renderingContext.setDefaultViewportScissor(cmdBuffer);

            graphicPipeline.bind(cmdBuffer);

            // Bind the bindless descriptor set
            cmdBuffer.getCommandBuffer().bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                graphicPipeline.getLayout(),
                0,
                {descriptorSet},
                {}
            );

            // Draw 4 quads, each with different texture and transform
            for (uint32_t i = 0; i < 4; i++) {
                PushConstants pc;
                pc.textureIndex = i;      // Each quad uses a different texture
                pc.transformIndex = i;    // Each quad uses a different transform

                cmdBuffer.getCommandBuffer().pushConstants(
                    graphicPipeline.getLayout(),
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(PushConstants),
                    &pc
                );

                // Draw the quad (6 vertices for 2 triangles)
                graphicPipeline.draw(cmdBuffer, static_cast<uint32_t>(indices.size()));
            }

            renderingContext.end(cmdBuffer);

            swapchainImage.prepareForPresentBarrier(cmdBuffer);

            cmdBuffer.end();

            // Submit
            vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submitInfo;
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

            device.presentImage(swapchainImage, {renderFinishedSemaphores[currentFrame]});

            currentFrame = (currentFrame + 1) % swapchainImageCount;
        }

        // Cleanup
        device.waitForAllCommands();

        for (size_t i = 0; i < swapchainImageCount; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }
    }

    device.releaseDevice();

    std::cout << "\nExample completed successfully!\n";
    std::cout << "Demonstrated bindless rendering with:\n";
    std::cout << "  - 4 textures in a bindless array\n";
    std::cout << "  - 4 transforms in a storage buffer array\n";
    std::cout << "  - Dynamic indexing via push constants\n";

    return 0;
}