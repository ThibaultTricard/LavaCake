#include <LavaCake/Device.hpp>
#include <LavaCake/Buffer.hpp>

#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <cassert>
#define assertm(exp, msg) assert((void(msg), exp))

// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------
int main()
{
   

    LavaCake::Device device(1,1);

    vk::CommandBuffer cmdBuffer = device.allocateCommandBuffer();

    std::vector<float> a;
    std::vector<float> b;

    int size = 1024;
    for(int i = 0; i < size; i++){
        a.push_back(i);
        b.push_back(i*2);
    }

    LavaCake::Buffer A(device, a, vk::BufferUsageFlagBits::eStorageBuffer ,  vk::AllocationCreateFlagBits::eCreateDedicatedMemory);
    LavaCake::Buffer B(device, b, vk::BufferUsageFlagBits::eStorageBuffer ,  vk::AllocationCreateFlagBits::eCreateDedicatedMemory);
    LavaCake::Buffer C(device, a.size()* sizeof(float), vk::BufferUsageFlagBits::eStorageBuffer , vk::AllocationCreateFlagBits::eCreateDedicatedMemory | vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

    auto pool = LavaCake::DescriptorPool::Builder(device)
                .setMaxSets(1)
                .addStorageBuffers(3)
                .build();

    auto layout = LavaCake::DescriptorSetLayout::Builder(device)
                            .addStorageBuffer(0,vk::ShaderStageFlagBits::eCompute)
                            .addStorageBuffer(1,vk::ShaderStageFlagBits::eCompute)
                            .addStorageBuffer(2,vk::ShaderStageFlagBits::eCompute).build();

    vk::DescriptorSet descriptorSet = pool.allocate(layout);

    LavaCake::DescriptorSetUpdater(device, descriptorSet)
                        .bindStorageBuffer(0, A)
                        .bindStorageBuffer(1, B)
                        .bindStorageBuffer(2, C)
                        .update();

    auto pipeline  = LavaCake::ComputePipeline::Builder(device)
                            .setShaderFromFile("../test.comp",LavaCake::ShadingLanguage::eGLSL)
                            .addDescriptorSetLayout(layout)
                            .build();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmdBuffer.begin(beginInfo);

    pipeline.bind(cmdBuffer);
    pipeline.bindDescriptorSets(cmdBuffer, {descriptorSet});
    pipeline.dispatch(cmdBuffer, size);

    cmdBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    auto queue = device.getAnyQueue();
    queue.submit(submitInfo);
    queue.waitIdle();
    
    void* data = C.map();
    std::vector<float> c(size);
    memcpy(c.data(), data, size*sizeof(float));
    for(int i = 0; i < size; i++){
        assertm(a[i] + b[i] == c[i], "a + b != c");
        std::cout<<c[i]<<", ";
    }

    std::cout<<std::endl;
                            

    return EXIT_SUCCESS;
}