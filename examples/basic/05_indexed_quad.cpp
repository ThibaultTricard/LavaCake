/**
 * LavaCake Example 05: Indexed Quad
 *
 * This example demonstrates:
 * - Creating a vertex buffer for quad vertices
 * - Creating an index buffer for indexed drawing
 * - Using drawIndexed() instead of draw()
 * - Efficient vertex reuse with index buffers
 *
 * A quad is drawn using 4 vertices and 6 indices (2 triangles).
 */

#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/Buffer.hpp>
#include <iostream>
#include <array>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// Vertex structure matching shader inputs
struct Vertex {
    float position[2];  // location 0: vec2
    float color[3];     // location 1: vec3
};

int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "05 - Indexed Quad Example", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device(surfaceConfig, 1);
    { // Create a context to make sure all GPU objects are destroyed before we release the device

        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Example 05: Indexed Quad\n";

        // Define quad vertices (4 corners)
        // The quad spans from (-0.5, -0.5) to (0.5, 0.5)
        std::vector<Vertex> vertices = {
            {{ -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }},  // Bottom-left  - Red
            {{  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }},  // Bottom-right - Green
            {{  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }},  // Top-right    - Blue
            {{ -0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f }}   // Top-left     - Yellow
        };

        // Define indices for two triangles forming a quad
        // Triangle 1: bottom-left, bottom-right, top-right
        // Triangle 2: bottom-left, top-right, top-left
        std::vector<uint16_t> indices = {
            0, 1, 2,  // First triangle
            0, 2, 3   // Second triangle
        };

        // Create vertex buffer
        LavaCake::Buffer vertexBuffer(
            device,
            vertices,
            vk::BufferUsageFlagBits::eVertexBuffer
        );

        // Create index buffer
        LavaCake::Buffer indexBuffer(
            device,
            indices,
            vk::BufferUsageFlagBits::eIndexBuffer
        );

        std::cout << "Vertex buffer created with " << vertices.size() << " vertices\n";
        std::cout << "Index buffer created with " << indices.size() << " indices\n";

        // Configure vertex input for the pipeline
        GraphicsPipeline::VertexInputInfo vertexInputInfo;

        // Binding description
        vertexInputInfo.bindings.push_back(vk::VertexInputBindingDescription{
            0,                                  // binding
            sizeof(Vertex),                     // stride
            vk::VertexInputRate::eVertex        // input rate
        });

        // Attribute descriptions
        vertexInputInfo.attributes.push_back(vk::VertexInputAttributeDescription{
            0,                                  // location
            0,                                  // binding
            vk::Format::eR32G32Sfloat,          // format (vec2)
            offsetof(Vertex, position)          // offset
        });

        vertexInputInfo.attributes.push_back(vk::VertexInputAttributeDescription{
            1,                                  // location
            0,                                  // binding
            vk::Format::eR32G32B32Sfloat,       // format (vec3)
            offsetof(Vertex, color)             // offset
        });

        // Create graphics pipeline with vertex input configuration
        LavaCake::GraphicsPipeline graphicPipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/vertex_buffer.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/triangle.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .addColorAttachmentFormat(device.getSwapchainFormat())
            .setVertexInput(vertexInputInfo)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .build();

        // Create semaphores per swapchain image
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        vk::SemaphoreCreateInfo semaphoreInfo;
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);

        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }

        uint32_t currentFrame = 0;

        // Main render loop
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            // Wait for previous frame to complete
            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            // Acquire next swapchain image
            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            // Begin recording commands
            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL
            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            // Begin dynamic rendering with dark gray background
            LavaCake::DynamicRenderingContext renderingContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage,
                    vk::ClearColorValue(std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.0f})
                )
                .begin(cmdBuffer);

            // Set the Viewport and Scissor
            renderingContext.setDefaultViewportScissor(cmdBuffer);

            // Bind the pipeline
            graphicPipeline.bind(cmdBuffer);

            // Bind the vertex buffer
            vk::DeviceSize vertexOffset = 0;
            cmdBuffer.getCommandBuffer().bindVertexBuffers(0, 1, vertexBuffer, &vertexOffset);

            // Bind the index buffer
            cmdBuffer.getCommandBuffer().bindIndexBuffer(
                indexBuffer,
                0,                          // offset
                vk::IndexType::eUint16      // index type matches std::vector<uint16_t>
            );

            // Draw indexed (6 indices = 2 triangles = 1 quad)
            graphicPipeline.drawIndexed(cmdBuffer, static_cast<uint32_t>(indices.size()));

            // End rendering
            renderingContext.end(cmdBuffer);

            // Transition swapchain image to PRESENT layout
            swapchainImage.prepareForPresentBarrier(cmdBuffer);

            cmdBuffer.end();

            // Submit command buffer
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

            // Present the image
            device.presentImage(swapchainImage, {renderFinishedSemaphores[currentFrame]});

            // Advance to next frame
            currentFrame = (currentFrame + 1) % swapchainImageCount;
        }

        // Wait for device to finish before cleanup
        device.waitForAllCommands();

        // Clean up semaphores
        for (size_t i = 0; i < swapchainImageCount; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }

        } // GPU objects destroyed here before device

        device.releaseDevice();
        
        std::cout << "Example completed successfully!\n";
        std::cout << "The quad was rendered using indexed drawing (4 vertices, 6 indices).\n";
   

    return 0;
}