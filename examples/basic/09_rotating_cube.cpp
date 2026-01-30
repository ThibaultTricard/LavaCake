/**
 * LavaCake Example 09: Rotating Cube
 *
 * This example demonstrates:
 * - Rendering a 3D cube with depth testing
 * - Using uniform buffers for MVP transformation matrices
 * - Programmable vertex pulling (bindless) for vertex data
 * - Creating and using a depth buffer
 * - Animating rotation over time using GLM
 *
 * The cube rotates continuously, showcasing 3D transformations with proper depth handling.
 */

#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/UniformBuffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>
#include <LavaCake/Image.hpp>
#include <iostream>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// Transformation matrices structure using GLM
struct TransformUBO {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "09 - Rotating Cube Example", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createWindowedDevice(surfaceConfig,1);
    { // Create a context to make sure all GPU objects are destroyed before we release the device

        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Example 09: Rotating Cube\n";

        // Define cube vertices (8 corners)
        // Each vertex has a unique color for visualization
        std::vector<float> positions = {
            // Front face
            -0.5f, -0.5f,  0.5f,  // 0: front-bottom-left
             0.5f, -0.5f,  0.5f,  // 1: front-bottom-right
             0.5f,  0.5f,  0.5f,  // 2: front-top-right
            -0.5f,  0.5f,  0.5f,  // 3: front-top-left
            // Back face
            -0.5f, -0.5f, -0.5f,  // 4: back-bottom-left
             0.5f, -0.5f, -0.5f,  // 5: back-bottom-right
             0.5f,  0.5f, -0.5f,  // 6: back-top-right
            -0.5f,  0.5f, -0.5f   // 7: back-top-left
        };

        // Vertex colors (one color per vertex)
        std::vector<float> colors = {
            1.0f, 0.0f, 0.0f,  // 0: Red
            0.0f, 1.0f, 0.0f,  // 1: Green
            0.0f, 0.0f, 1.0f,  // 2: Blue
            1.0f, 1.0f, 0.0f,  // 3: Yellow
            1.0f, 0.0f, 1.0f,  // 4: Magenta
            0.0f, 1.0f, 1.0f,  // 5: Cyan
            1.0f, 0.5f, 0.0f,  // 6: Orange
            0.5f, 0.0f, 1.0f   // 7: Purple
        };

        // Indices for 12 triangles (6 faces * 2 triangles each)
        std::vector<uint32_t> indices = {
            // Front face
            0, 1, 2,  0, 2, 3,
            // Back face
            5, 4, 7,  5, 7, 6,
            // Top face
            3, 2, 6,  3, 6, 7,
            // Bottom face
            4, 5, 1,  4, 1, 0,
            // Right face
            1, 5, 6,  1, 6, 2,
            // Left face
            4, 0, 3,  4, 3, 7
        };

        // Create storage buffers for vertex data
        LavaCake::Buffer positionBuffer(
            device,
            positions,
            vk::BufferUsageFlagBits::eStorageBuffer
        );

        LavaCake::Buffer colorBuffer(
            device,
            colors,
            vk::BufferUsageFlagBits::eStorageBuffer
        );

        LavaCake::Buffer indexBuffer(
            device,
            indices,
            vk::BufferUsageFlagBits::eStorageBuffer
        );

        std::cout << "Storage buffers created for cube geometry\n";

        // Create uniform buffer for transformation matrices
        LavaCake::UniformBuffer transformBuffer(device);
        TransformUBO transform{};
        transform.model = glm::mat4(1.0f);
        transform.view = glm::mat4(1.0f);
        transform.projection = glm::mat4(1.0f);
        transformBuffer.addVariable("transform", transform);
        transformBuffer.end();

        std::cout << "Uniform buffer created for transformations\n";

        // Create depth buffer
        vk::Extent2D extent = device.getSwapchainExtent();
        vk::Format depthFormat = vk::Format::eD32Sfloat;

        LavaCake::Image depthImage(
            device,
            extent.width,
            extent.height,
            1,  // depth
            depthFormat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment
        );

        LavaCake::ImageView depthImageView(
            depthImage,
            vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eDepth
        );

        std::cout << "Depth buffer created\n";

        // Create descriptor set layout
        LavaCake::DescriptorSetLayout descriptorSetLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)  // positions
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)  // colors
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eVertex)  // indices
                .addBinding(3, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)  // transform
                .build();

        // Create descriptor pool
        LavaCake::DescriptorPool descriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(3)
                .addUniformBuffers(1)
                .setMaxSets(1)
                .build();

        // Allocate descriptor set
        vk::DescriptorSet descriptorSet = descriptorPool.allocate(descriptorSetLayout);

        // Update descriptor set with our buffers
        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, positionBuffer)
            .bindStorageBuffer(1, colorBuffer)
            .bindStorageBuffer(2, indexBuffer)
            .bindUniformBuffer(3, transformBuffer, 0, transformBuffer.size())
            .update();

        std::cout << "Descriptor set updated\n";

        // Create graphics pipeline with depth testing enabled
        LavaCake::GraphicsPipeline graphicPipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/rotating_cube.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/rotating_cube.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .addColorAttachmentFormat(device.getSwapchainFormat())
            .setDepthAttachmentFormat(depthFormat)
            .setDepthTest(true, true, vk::CompareOp::eLess)
            .setBindlessVertexInput()
            .addDescriptorSetLayout(descriptorSetLayout)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .build();

        std::cout << "Graphics pipeline created with depth testing\n";

        // Create semaphores per swapchain image
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        vk::SemaphoreCreateInfo semaphoreInfo;
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);

        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }

        // Transition depth image to optimal layout (one-time operation)
        {
            LavaCake::CommandBuffer cmd(device);
            cmd.begin();

            depthImage.transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

            cmd.end();
            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = cmd;
            device.getGraphicQueue(0).submit(submitInfo);
            device.getGraphicQueue(0).waitIdle();
        }

        float angle = 0.0f;
        uint32_t currentFrame = 0;

        std::cout << "Starting render loop...\n";

        // Main render loop
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            // Wait for previous frame to complete
            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            // Update transformation matrices using GLM
            angle += 0.01f;

            // Create model matrix with rotation around Y and X axes
            transform.model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
            transform.model = glm::rotate(transform.model, angle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));

            // Set up view matrix (camera at (0, 0, 3) looking at origin)
            transform.view = glm::lookAt(
                glm::vec3(0.0f, 0.0f, 3.0f),  // eye position
                glm::vec3(0.0f, 0.0f, 0.0f),  // center
                glm::vec3(0.0f, 1.0f, 0.0f)   // up vector
            );

            // Set up perspective projection (GLM_FORCE_DEPTH_ZERO_TO_ONE handles Vulkan depth range)
            float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
            transform.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
            transform.projection[1][1] *= -1;  // Flip Y for Vulkan coordinate system

            transformBuffer.setVariable("transform", transform);

            // Acquire next swapchain image
            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            // Begin recording commands
            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // Update uniform buffer on GPU
            transformBuffer.update(cmdBuffer);

            // Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL
            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            // Begin dynamic rendering with depth attachment
            LavaCake::DynamicRenderingContext renderingContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage,
                    vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.15f, 1.0f})
                )
                .setDepthAttachment(depthImageView, 1.0f)
                .begin(cmdBuffer);

            // Set the Viewport and Scissor
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

            // Draw 36 vertices (12 triangles * 3 vertices each)
            graphicPipeline.draw(cmdBuffer, static_cast<uint32_t>(indices.size()));

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
    std::cout << "The cube was rendered with rotation animation and depth testing.\n";

    return 0;
}
