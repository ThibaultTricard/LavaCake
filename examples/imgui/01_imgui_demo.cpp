/**
 * LavaCake Example: ImGui Demo
 *
 * This example demonstrates:
 * - Integrating ImGui with LavaCake
 * - Creating an ImGuiRenderer instance
 * - Rendering ImGui UI elements with dynamic rendering
 * - Basic ImGui window with demo content
 *
 * Note: ImGuiRenderer is window-manager agnostic. You handle the platform
 * backend (GLFW, SDL2, etc.) yourself, and ImGuiRenderer handles Vulkan rendering.
 */

#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/ImGui.hpp>

// ImGui GLFW backend (you choose your platform backend)
#include <imgui_impl_glfw.h>

#include <iostream>

using namespace LavaCake;

int main() {
    // Create a device with a window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "LavaCake ImGui Demo", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device(surfaceConfig, 1);
    {
        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Example: ImGui Demo\n";

        // Create ImGui renderer (shaders are embedded in the library)
        LavaCake::ImGuiRenderer imgui(device, device.getSwapchainFormat());

        // Initialize GLFW backend (user's responsibility - platform agnostic)
        ImGui::SetCurrentContext(imgui.getContext());
        ImGui_ImplGlfw_InitForVulkan(window, true);

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
        bool showDemoWindow = true;

        // Main render loop
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // Wait for previous frame to complete
            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            // Start new ImGui frame (platform backend first, then our renderer)
            ImGui_ImplGlfw_NewFrame();
            imgui.newFrame();

            // Build ImGui UI
            if (showDemoWindow) {
                ImGui::ShowDemoWindow(&showDemoWindow);
            }

            // Custom window
            ImGui::Begin("LavaCake ImGui Integration");
            ImGui::Text("Welcome to LavaCake + ImGui!");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                       1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Checkbox("Show Demo Window", &showDemoWindow);
            ImGui::End();

            // Acquire next swapchain image
            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            // Begin recording commands
            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL
            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            // Begin dynamic rendering
            LavaCake::DynamicRenderingContext renderingContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage,
                    vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}) // Dark gray background
                )
                .begin(cmdBuffer);

            // Set the Viewport and Scissor for rendering on the whole image
            renderingContext.setDefaultViewportScissor(cmdBuffer);

            // Render ImGui
            imgui.render(cmdBuffer);

            // End rendering
            renderingContext.end(cmdBuffer);

            // Transition swapchain image to PRESENT layout
            swapchainImage.prepareForPresentBarrier(cmdBuffer);

            cmdBuffer.end();

            // Submit command buffer with fence
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

        // Shutdown GLFW backend before ImGuiRenderer destructor
        ImGui_ImplGlfw_Shutdown();

        // Clean up semaphores
        for (size_t i = 0; i < swapchainImageCount; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }
    }

    device.releaseDevice();
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Example completed successfully!\n";
    return 0;
}
