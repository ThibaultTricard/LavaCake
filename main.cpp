#include "Library/Lavacake/Device.hpp"
#include "Library/Lavacake/Buffer.hpp"

#include "Library/Lavacake/ShaderModule.hpp"

// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------
int main()
{
   
    // -----------------------------------------------------------
    // 1) Initialize GLFW
    // -----------------------------------------------------------
    /*glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vulkan-Hpp Full Example", nullptr, nullptr);*/

    // -----------------------------------------------------------
    // 1) Create Device
    // -----------------------------------------------------------

    LavaCake::Device device(1,2);

    // ---------------------------------------------------------
    // 2) Allocate a command buffer
    // ---------------------------------------------------------
    vk::CommandBuffer cmdBuffer = device.allocateCommandBuffer();


    std::vector<float> in;

    for(int i = 0; i < 1000; i++){
        in.push_back(i);
    }

    LavaCake::Buffer bufferIn(device,in, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,  vk::AllocationCreateFlagBits::eCreateDedicatedMemory);

    LavaCake::Buffer bufferOut(device,in.size()* sizeof(float),vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::AllocationCreateFlagBits::eCreateDedicatedMemory | vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmdBuffer.begin(beginInfo);

    bufferIn.copyToBuffer(cmdBuffer,bufferOut);

    cmdBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    auto queue = device.getAnyQueue();
    queue.submit(submitInfo);
    queue.waitIdle();

    void* data = bufferOut.map();
    std::vector<float> out(1000);
    memcpy(out.data(), data, 1000*sizeof(float));

    for(int i = 0; i < 1000; i++){
        std::cout<<out[i]<<", ";
    }

    std::cout<<std::endl;


    auto computeModule = LavaCake::createShaderModuleFromGLSL(device,"../test.comp", vk::ShaderStageFlagBits::eCompute);
    


    // -----------------------------------------------------------
    // MAIN LOOP
    // -----------------------------------------------------------
    /*while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }*/

    // -----------------------------------------------------------
    // Cleanup
    // -----------------------------------------------------------
    

    //glfwDestroyWindow(window);
    //glfwTerminate();
    
    

    return EXIT_SUCCESS;
}