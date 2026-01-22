/**
 * LavaCake Example 02: Colored Triangle
 *
 * This example demonstrates:
 * - Creating and compiling shaders from GLSL files
 * - Building a graphics pipeline
 * - Drawing a simple triangle with vertex colors
 * - Basic rendering with dynamic rendering
 *
 * The triangle vertices and colors are hardcoded in the vertex shader.
 */

#include <LavaCake/Device.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <iostream>
#include <thread>
#include <chrono>


std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "02 - Colored Triangle Example", nullptr, nullptr);

    LavaCake::Device device(window,1);


    vk::CommandBuffer cmdBuffer = device.allocateCommandBuffer();

    std::cout << "LavaCake Example 02: Colored Triangle\n";

    // Graphic Pipeline creation

    LavaCake::GraphicsPipeline graphicPipeline  = LavaCake::GraphicsPipeline::Builder(device)
                        .addShaderFromFile(root + "shaders/triangle.vert",vk::ShaderStageFlagBits::eVertex,LavaCake::ShadingLanguage::eGLSL)
                        .addShaderFromFile(root + "shaders/triangle.frag",vk::ShaderStageFlagBits::eFragment,LavaCake::ShadingLanguage::eGLSL)
                        .addColorAttachmentFormat(device.getSwapchainFormat())
                        .setCullMode(vk::CullModeFlagBits::eNone)
                        .build();

    // Create semaphores for synchronization
    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();


        // Acquire next swapchain image
        imageAvailableSemaphore = device.getDevice().createSemaphore(semaphoreInfo);
        LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphore);

        // Get command buffer for this frame
        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        cmdBuffer.begin(beginInfo);

        // Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL
        swapchainImage.prepareForAttachementBarrier(cmdBuffer);

        // Begin dynamic rendering with cornflower blue clear color
        LavaCake::DynamicRenderingContext renderingContext = LavaCake::DynamicRenderingContext::Builder()
            .setRenderArea(device.getSwapchainExtent())
            .addColorAttachment(
                swapchainImage.getView(),
                vk::ClearColorValue(std::array<float, 4>{0.39f, 0.58f, 0.93f, 1.0f}) // Cornflower blue!
            )
            .begin(cmdBuffer);
        
        // Set the Viewport and Scissor to for rendering on the whole image
        renderingContext.setDefaultViewportScissor(cmdBuffer);

        // binding the pipeline
        graphicPipeline.bind(cmdBuffer);

        // draw call for three vertices
        graphicPipeline.draw(cmdBuffer,3);

        // End rendering
        renderingContext.end(cmdBuffer);

        // Transition swapchain image to PRESENT layout
        swapchainImage.prepareForPresentBarrier(cmdBuffer);

        cmdBuffer.end();


        renderFinishedSemaphore = device.getDevice().createSemaphore(semaphoreInfo);
        // Submit command buffer and wait for image to be available
        vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submitInfo;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

        device.getGraphicQueue(0).submit(submitInfo);

        // Present the image
        device.presentImage(swapchainImage, {renderFinishedSemaphore});


        device.getGraphicQueue(0).waitIdle();

        device.getDevice().destroySemaphore(imageAvailableSemaphore);
        device.getDevice().destroySemaphore(renderFinishedSemaphore);

    }

    // Wait for device to finish before cleanup
    device.waitForAllCommands();

    std::cout << "Example completed successfully!\n";
    return 0;

    
}