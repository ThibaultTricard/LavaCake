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
 *
 * The triangle color cycles through the rainbow using a uniform buffer.
 */

#include <LavaCake/Device.hpp>
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

    vk::CommandBuffer cmdBuffer = device.allocateCommandBuffer();

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

    // Create semaphores for synchronization
    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;

    float time = 0.0f;

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Update uniform buffer with new color (cycling through rainbow)
        ColorUBO newColor;
        newColor.r = (std::sin(time) + 1.0f) * 0.5f;
        newColor.g = (std::sin(time + 2.0f) + 1.0f) * 0.5f;
        newColor.b = (std::sin(time + 4.0f) + 1.0f) * 0.5f;
        newColor.time = time;

        uniformBuffer.setVariable("color", newColor);

        time += 0.01f;

        // Acquire next swapchain image
        imageAvailableSemaphore = device.getDevice().createSemaphore(semaphoreInfo);
        LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphore);

        // Get command buffer for this frame
        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        cmdBuffer.begin(beginInfo);

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
        cmdBuffer.bindDescriptorSets(
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

        renderFinishedSemaphore = device.getDevice().createSemaphore(semaphoreInfo);

        // Submit command buffer
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
    std::cout << "The triangle color cycled through the rainbow using uniform buffers.\n";

    return 0;
}
