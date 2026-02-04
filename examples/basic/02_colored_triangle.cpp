/**
 * LavaCake Example 02: Colored Triangle
 *
 * This example demonstrates:
 * - Creating and compiling shaders from GLSL files
 * - Building a graphics pipeline
 * - Drawing a simple triangle with vertex colors
 * - Basic rendering with dynamic rendering
 * - Using CommandBuffer with fence for GPU synchronization
 *
 * The triangle vertices and colors are hardcoded in the vertex shader.
 */

#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
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

    bool framebufferResized = false;
    glfwSetWindowUserPointer(window, &framebufferResized);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int, int) {
        *static_cast<bool*>(glfwGetWindowUserPointer(win)) = true;
    });

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig,1);
    { // Create a context to make sure all GPU objects are destroyed before we release the device

        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Example 02: Colored Triangle\n";

        // Graphic Pipeline creation

        LavaCake::GraphicsPipeline graphicPipeline  = LavaCake::GraphicsPipeline::Builder(device)
                            .addShaderFromFile(root + "shaders/triangle.vert",vk::ShaderStageFlagBits::eVertex,LavaCake::ShadingLanguage::eGLSL)
                            .addShaderFromFile(root + "shaders/triangle.frag",vk::ShaderStageFlagBits::eFragment,LavaCake::ShadingLanguage::eGLSL)
                            .addColorAttachmentFormat(device.getSwapchainFormat())
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

            if (framebufferResized) {
                framebufferResized = false;
                // Spin while minimized — swapchain can't be created with a zero extent
                int width = 0, height = 0;
                glfwGetFramebufferSize(window, &width, &height);
                while (width == 0 || height == 0) {
                    glfwWaitEvents();
                    glfwGetFramebufferSize(window, &width, &height);
                }

                auto newExtent = vk::Extent2D(width,height);
                device.resizeSwapchain(newExtent);
                continue;
            }

            // Wait for previous frame to complete
            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            try {
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
                        swapchainImage,
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

            } catch (const vk::OutOfDateKHRError&) {
                // Swapchain became incompatible mid-frame (resize arrived after the
                // flag check). Trigger the resize path on the next iteration;
                // resizeSwapchain will waitIdle before recreating.
                framebufferResized = true;
                continue;
            }

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
    } // GPU objects destroyed here before device

    device.releaseDevice();
    std::cout << "Example completed successfully!\n";
    

    return 0;
}