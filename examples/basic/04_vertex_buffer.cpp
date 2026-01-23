/**
 * LavaCake Example 04: Vertex Buffer
 *
 * This example demonstrates:
 * - Creating a vertex buffer using the Buffer class
 * - Defining vertex input bindings and attributes
 * - Configuring the graphics pipeline for vertex input
 * - Binding vertex buffers and drawing
 *
 * The triangle vertices and colors come from a GPU vertex buffer
 * instead of being hardcoded in the shader.
 */

#include <LavaCake/Device.hpp>
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "04 - Vertex Buffer Example", nullptr, nullptr);

    LavaCake::Device device(window, 1);

    // Create command buffer with fence for synchronization
    LavaCake::CommandBuffer cmdBuffer(device, true);

    std::cout << "LavaCake Example 04: Vertex Buffer\n";

    // Define triangle vertices with position and color
    std::vector<Vertex> vertices = {
        {{  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},  // Top vertex - Red
        {{  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }},  // Bottom right - Green
        {{ -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }}   // Bottom left - Blue
    };

    // Create vertex buffer using the Buffer class
    LavaCake::Buffer vertexBuffer(
        device,
        vertices,
        vk::BufferUsageFlagBits::eVertexBuffer
    );

    std::cout << "Vertex buffer created with " << vertices.size() << " vertices\n";

    // Configure vertex input for the pipeline
    GraphicsPipeline::VertexInputInfo vertexInputInfo;

    // Binding description: one binding at binding 0, with per-vertex input rate
    vertexInputInfo.bindings.push_back(vk::VertexInputBindingDescription{
        0,                                  // binding
        sizeof(Vertex),                     // stride
        vk::VertexInputRate::eVertex        // input rate
    });

    // Attribute descriptions
    // Location 0: position (vec2 at offset 0)
    vertexInputInfo.attributes.push_back(vk::VertexInputAttributeDescription{
        0,                                  // location
        0,                                  // binding
        vk::Format::eR32G32Sfloat,          // format (vec2)
        offsetof(Vertex, position)          // offset
    });

    // Location 1: color (vec3 at offset 8)
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

    // Create semaphores per swapchain image to avoid reuse conflicts
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

        // Acquire next swapchain image using current frame's semaphore
        LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

        // Begin recording commands
        cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        // Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL
        swapchainImage.prepareForAttachementBarrier(cmdBuffer);

        // Begin dynamic rendering with cornflower blue clear color
        LavaCake::DynamicRenderingContext renderingContext = LavaCake::DynamicRenderingContext::Builder()
            .setRenderArea(device.getSwapchainExtent())
            .addColorAttachment(
                swapchainImage.getView(),
                vk::ClearColorValue(std::array<float, 4>{0.39f, 0.58f, 0.93f, 1.0f})
            )
            .begin(cmdBuffer);

        // Set the Viewport and Scissor for rendering on the whole image
        renderingContext.setDefaultViewportScissor(cmdBuffer);

        // Bind the pipeline
        graphicPipeline.bind(cmdBuffer);

        // Bind the vertex buffer
        vk::Buffer vkBuffer = vertexBuffer.getBuffer();
        vk::DeviceSize offset = 0;
        cmdBuffer.getCommandBuffer().bindVertexBuffers(0, 1, &vkBuffer, &offset);

        // Draw the triangle (3 vertices)
        graphicPipeline.draw(cmdBuffer, static_cast<uint32_t>(vertices.size()));

        // End rendering
        renderingContext.end(cmdBuffer);

        // Transition swapchain image to PRESENT layout
        swapchainImage.prepareForPresentBarrier(cmdBuffer);

        cmdBuffer.end();

        // Submit command buffer with fence using current frame's semaphores
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

    // Clean up all semaphores
    for (size_t i = 0; i < swapchainImageCount; i++) {
        device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
        device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
    }

    std::cout << "Example completed successfully!\n";
    std::cout << "The triangle was rendered using vertex data from a GPU buffer.\n";

    return 0;
}