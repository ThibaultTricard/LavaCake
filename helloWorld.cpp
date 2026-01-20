#include "Library/LavaCake/Device.hpp"
#include "Library/LavaCake/Buffer.hpp"
#include "Library/LavaCake/ShaderModule.hpp"

#include "Library/LavaCake/ComputePipeline.hpp"
#include "Library/LavaCake/DescriptorSet.hpp"
#include "Library/LavaCake/DescriptorPool.hpp"

#include "Library/LavaCake/GraphicPipeline.hpp"
#include "Library/LavaCake/DynamicRendering.hpp"

#include <cassert>
#define assertm(exp, msg) assert((void(msg), exp))

// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------
int main()
{
   
    // -----------------------------------------------------------
    // 1) Initialize GLFW
    // -----------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "HelloWorld", nullptr, nullptr);

    LavaCake::Device device(window,1,1);

    vk::CommandBuffer cmdBuffer = device.allocateCommandBuffer();
    

    auto pipeline  = LavaCake::GraphicsPipeline::Builder(device)
                            .addShaderFromFile("../shader.vert",vk::ShaderStageFlagBits::eVertex,LavaCake::ShadingLanguage::eGLSL)
                            .addShaderFromFile("../shader.frag",vk::ShaderStageFlagBits::eFragment,LavaCake::ShadingLanguage::eGLSL)
                            .addColorAttachmentFormat(device.getSwapchainFormat())
                            .setCullMode(vk::CullModeFlagBits::eNone)
                            .build();

    
    vk::SemaphoreCreateInfo semInfo = {};
    vk::Semaphore SwapChainSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    // -----------------------------------------------------------
    // MAIN LOOP
    // -----------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        cmdBuffer.begin(beginInfo);
                        
        SwapChainSemaphore = device.getDevice().createSemaphore(semInfo);
        auto colorImage = device.aquireSwapChainImage(SwapChainSemaphore);
        vk::PipelineStageFlags waitingStage = vk::PipelineStageFlagBits::eFragmentShader;
        

        colorImage.prepareForAttachementBarrier(cmdBuffer);

        auto rendering = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(1280, 720)
                .addColorAttachment(colorImage.getView())
                .begin(cmdBuffer);

        rendering.setDefaultViewportScissor(cmdBuffer);
        

        pipeline.bind(cmdBuffer);
        pipeline.draw(cmdBuffer,3);

        rendering.end(cmdBuffer);

        colorImage.prepareForPresentBarrier(cmdBuffer);
        cmdBuffer.end();
        

        renderFinishedSemaphore = device.getDevice().createSemaphore(semInfo);
        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.pWaitSemaphores = &SwapChainSemaphore;
        submitInfo.waitSemaphoreCount=1;
        submitInfo.pWaitDstStageMask = &waitingStage;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;


        auto queue = device.getGraphicQueue(0);
        
        queue.submit(submitInfo);
        device.presentImage(colorImage,{renderFinishedSemaphore});
        queue.waitIdle();

        device.getDevice().destroySemaphore(SwapChainSemaphore);
        device.getDevice().destroySemaphore(renderFinishedSemaphore);
    }

    // -----------------------------------------------------------
    // Cleanup
    // -----------------------------------------------------------
    

    glfwDestroyWindow(window);
    glfwTerminate();
    
    

    return EXIT_SUCCESS;
}