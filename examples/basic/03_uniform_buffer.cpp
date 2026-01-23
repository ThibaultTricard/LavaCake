/**
 * LavaCake Example 03: Uniform Buffer
 *
 * This example demonstrates:
 * - Creating and using UniformBuffer
 * - Adding variables to a uniform buffer
 * - Updating uniform buffer data each frame
 * - Binding uniform buffers to shaders
 * - Creating descriptor sets for uniform buffers
 * - Animating colors using uniforms
 * - Using CommandBuffer with fence for GPU synchronization
 *
 * The triangle color cycles through the rainbow using a uniform buffer.
 */

#include <LavaCake/Device.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/UniformBuffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>
#include <iostream>
#include <cmath>
#include <array>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// Simple structure for our uniform data
struct ColorUBO {
    float r;
    float g;
    float b;
    float time;
};

int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "03 - Uniform Buffer Example", nullptr, nullptr);

    LavaCake::Device device(window, 1);

    // Create command buffer with fence for synchronization
    LavaCake::CommandBuffer cmdBuffer(device, true);

    std::cout << "LavaCake Example 03: Uniform Buffer\n";

    // Create uniform buffer
    LavaCake::UniformBuffer uniformBuffer(device);

    // Add variables to the uniform buffer
    ColorUBO initialColor = {1.0f, 0.0f, 0.0f, 0.0f};
    uniformBuffer.addVariable("color", initialColor);

    // Finalize the uniform buffer (allocates GPU memory)
    uniformBuffer.end();

    // Create descriptor set layout
    LavaCake::DescriptorSetLayout descriptorSetLayout =
        LavaCake::DescriptorSetLayout::Builder(device)
            .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
            .build();

    // Create descriptor pool
    LavaCake::DescriptorPool descriptorPool =
        LavaCake::DescriptorPool::Builder(device)
            .addUniformBuffers(1)
            .setMaxSets(1)
            .build();

    // Allocate descriptor set
    vk::DescriptorSet descriptorSet = descriptorPool.allocate(descriptorSetLayout);

    // Update descriptor set to point to our uniform buffer
    LavaCake::DescriptorSetUpdater(device, descriptorSet)
        .bindUniformBuffer(0, uniformBuffer.getBuffer(), 0, uniformBuffer.size())
        .update();

    // Create graphics pipeline with descriptor set
    LavaCake::GraphicsPipeline graphicPipeline =
        LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/uniform_triangle.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/uniform_triangle.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .addColorAttachmentFormat(device.getSwapchainFormat())
            .setCullMode(vk::CullModeFlagBits::eNone)
            .addDescriptorSetLayout(descriptorSetLayout)
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

    float time = 0.0f;
    uint32_t currentFrame = 0;

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Wait for previous frame to complete
        cmdBuffer.waitForCompletion();
        cmdBuffer.reset();

        // Update uniform buffer with new color (cycling through rainbow)
        ColorUBO newColor;
        newColor.r = (std::sin(time) + 1.0f) * 0.5f;
        newColor.g = (std::sin(time + 2.0f) + 1.0f) * 0.5f;
        newColor.b = (std::sin(time + 4.0f) + 1.0f) * 0.5f;
        newColor.time = time;

        uniformBuffer.setVariable("color", newColor);

        time += 0.01f;

        // Acquire next swapchain image using current frame's semaphore
        LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

        // Begin recording commands
        cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        // Update uniform buffer on GPU
        uniformBuffer.update(cmdBuffer);

        // Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL
        swapchainImage.prepareForAttachementBarrier(cmdBuffer);

        // Begin dynamic rendering with dark background
        LavaCake::DynamicRenderingContext renderingContext =
            LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage.getView(),
                    vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f})
                )
                .begin(cmdBuffer);

        // Set the Viewport and Scissor for rendering on the whole image
        renderingContext.setDefaultViewportScissor(cmdBuffer);

        // Bind the pipeline
        graphicPipeline.bind(cmdBuffer);

        // Bind descriptor set
        cmdBuffer.getCommandBuffer().bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            graphicPipeline.getLayout(),
            0,
            {descriptorSet},
            {}
        );

        // Draw call for three vertices
        graphicPipeline.draw(cmdBuffer, 3);

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
    std::cout << "The triangle color cycled through the rainbow using uniform buffers.\n";

    return 0;
}
