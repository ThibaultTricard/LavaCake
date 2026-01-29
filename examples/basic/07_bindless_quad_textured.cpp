/**
 * LavaCake Example 07: Bindless Quad (Programmable Vertex Pulling) Textured
 *
 * This example demonstrates:
 * - Using storage buffers instead of vertex/index buffers
 * - Binding vertex data via descriptor sets
 * - Programmable vertex pulling using gl_VertexIndex
 * - Modern bindless rendering approach
 * - Texture binding
 *
 * Instead of using traditional vertex input bindings, vertex data is stored
 * in storage buffers and fetched manually in the vertex shader using gl_VertexIndex.
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

std::string root = PROJECT_ROOT;

using namespace LavaCake;

int main() {

    // Create a device with a 800x600 window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "07 - Bindless Quad Example Textured", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createAdvancedDevice(surfaceConfig,1);
    { // Create a context to make sure all GPU objects are destroyed before we release the device

        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Example 07: Bindless Quad (Programmable Vertex Pulling) Textured\n";

        // Vertex positions (vec2) - 4 corners of the quad
        std::vector<float> positions = {
            -0.5f, -0.5f,  // Bottom-left
            0.5f, -0.5f,  // Bottom-right
            0.5f,  0.5f,  // Top-right
            -0.5f,  0.5f   // Top-left
        };

        // Vertex UV (vec2) - one per vertex
        std::vector<float> UV = {
            0.0f, 0.0f,  
            0.0f, 1.0f,  
            1.0f, 1.0f,  
            1.0f, 0.0f, 
        };

        // Indices - 6 indices for 2 triangles
        std::vector<uint32_t> indices = {
            0, 1, 2,  // First triangle
            0, 2, 3   // Second triangle
        };

        // Create storage buffers for vertex data
        LavaCake::Buffer positionBuffer(
            device,
            positions,
            vk::BufferUsageFlagBits::eStorageBuffer
        );

        LavaCake::Buffer UVBuffer(
            device,
            UV,
            vk::BufferUsageFlagBits::eStorageBuffer
        );

        LavaCake::Buffer indexBuffer(
            device,
            indices,
            vk::BufferUsageFlagBits::eStorageBuffer
        );

        std::cout << "Storage buffers created for positions, textureCoordinates, and indices\n";


        // Create the image data
        uint32_t imageSize = 64;
        uint32_t cellSize = 4;
        std::vector<float> imageData(imageSize*imageSize);

        // Proceduraly create a checker board can be replace by reading a texture file
        for(int i = 0; i < imageSize; i++){
            for(int j = 0; j < imageSize; j++){
                float color = 0;
                if(i %(cellSize*2) < cellSize){
                    if(j %(cellSize*2) < cellSize){
                        imageData[i + j * imageSize] = 0;
                    }else{
                        imageData[i + j * imageSize] = 1;
                    }
                }else{
                    if(j %(cellSize*2) < cellSize){
                        imageData[i + j * imageSize] = 1;
                    }else{
                        imageData[i + j * imageSize] = 0;
                    }
                }
            }
        }

        // Creation of the Image on the GPU
        LavaCake::Image checkerImage(device, imageData, imageSize,imageSize, 1, vk::Format::eR32Sfloat, vk::ImageUsageFlagBits::eSampled);
        
        // Creation of Image View
        LavaCake::ImageView checkerImageView(checkerImage);

        // Creation of a Sampler
        LavaCake::Sampler checkerSampler(device);


        std::cout << "Image, ImageView and Sampler created. \n";


        // Create descriptor set layout with 3 storage buffer an a texture (CombinedImageSampler) bindings
        LavaCake::DescriptorSetLayout descriptorSetLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)  // positions
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)  // UV
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eVertex)  // indices
                .addCombinedImageSampler(3, vk::ShaderStageFlagBits::eFragment) // Texture
                .build();

        // Create descriptor pool
        LavaCake::DescriptorPool descriptorPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(3)
                .addCombinedImageSamplers(1)
                .setMaxSets(1)
                .build();

        // Allocate descriptor set
        vk::DescriptorSet descriptorSet = descriptorPool.allocate(descriptorSetLayout);

        // Update descriptor set with our storage buffers and our texture
        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, positionBuffer)
            .bindStorageBuffer(1, UVBuffer)
            .bindStorageBuffer(2, indexBuffer)
            .bindImage(3, checkerImageView, checkerSampler)
            .update();

        std::cout << "Descriptor set updated with storage buffers and a texture\n";

        // Create graphics pipeline - note: no vertex input configuration needed!
        LavaCake::GraphicsPipeline graphicPipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/bindless_quad_textured.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/bindless_quad_textured.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .addColorAttachmentFormat(device.getSwapchainFormat())
            .setBindlessVertexInput()  // No vertex input - we fetch from storage buffers
            .addDescriptorSetLayout(descriptorSetLayout)
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
                    vk::ClearColorValue(std::array<float, 4>{0.15f, 0.15f, 0.2f, 1.0f})
                )
                .begin(cmdBuffer);

            // Set the Viewport and Scissor
            renderingContext.setDefaultViewportScissor(cmdBuffer);

            // Bind the pipeline
            graphicPipeline.bind(cmdBuffer);

            // Bind descriptor set (contains our vertex data in storage buffers)
            cmdBuffer.getCommandBuffer().bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                graphicPipeline.getLayout(),
                0,
                {descriptorSet},
                {}
            );

            // Draw 6 vertices (the shader will use gl_VertexIndex to fetch from index buffer)
            // No vertex buffer binding needed!
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
    std::cout << "The quad was rendered using programmable vertex pulling from storage buffers and textured with a sampler2D.\n";
   

    return 0;
}
