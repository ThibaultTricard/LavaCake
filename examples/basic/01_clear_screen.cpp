/**
 * LavaCake Example 01: Clear Screen
 *
 * This is the simplest possible LavaCake example. It demonstrates:
 * - Creating a Device with a window
 * - Setting up dynamic rendering
 * - Clearing the screen to a color
 * - Basic render loop and swapchain presentation
 *
 * No shaders or pipelines needed - just clears to cornflower blue!
 */

#include <LavaCake/Device.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <iostream>
#include <thread>
#include <chrono>


int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "01 - Clear Screen Example", nullptr, nullptr);

    LavaCake::Device device(window,1);


    vk::CommandBuffer cmdBuffer = device.allocateCommandBuffer();

    std::cout << "LavaCake Example 01: Clear Screen\n";

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
        // Nothing to draw - just clearing!

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