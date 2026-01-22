/**
 * LavaCake Example: Vector Addition (Compute)
 *
 * This example demonstrates:
 * - Creating a headless device (no window)
 * - Setting up a compute pipeline
 * - Using storage buffers for GPU computation
 * - Computing C = A + B on the GPU
 * - Reading results back to CPU
 */

#include <LavaCake/Device.hpp>
#include <LavaCake/Buffer.hpp>

#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/DescriptorPool.hpp>

#include <cassert>
#define assertm(exp, msg) assert((void(msg), exp))

std::string root = PROJECT_ROOT;


int main() {
    try {
        // Create a headless device (no window needed for compute)
        LavaCake::Device device(0, 1);

        std::cout << "LavaCake Compute Example: Vector Addition\n";
        std::cout << "Computing C = A + B on the GPU\n\n";

        // Create input data
        const uint32_t size = 1024;
        std::vector<float> a(size);
        std::vector<float> b(size);

        for (uint32_t i = 0; i < size; i++) {
            a[i] = static_cast<float>(i);
            b[i] = static_cast<float>(i * 2);
        }

        std::cout << "Input size: " << size << " elements\n";
        std::cout << "A[0]=" << a[0] << ", B[0]=" << b[0] << "\n";
        std::cout << "A[" << size-1 << "]=" << a[size-1] << ", B[" << size-1 << "]=" << b[size-1] << "\n\n";

        // Create GPU buffers
        LavaCake::Buffer bufferA(device, a,
                      vk::BufferUsageFlagBits::eStorageBuffer,
                      vk::AllocationCreateFlagBits::eCreateDedicatedMemory);

        LavaCake::Buffer bufferB(device, b,
                      vk::BufferUsageFlagBits::eStorageBuffer,
                      vk::AllocationCreateFlagBits::eCreateDedicatedMemory);

        // Output buffer needs to be host-visible so we can read it back
        LavaCake::Buffer bufferC(device, size * sizeof(float),
                      vk::BufferUsageFlagBits::eStorageBuffer,
                      vk::AllocationCreateFlagBits::eCreateDedicatedMemory |
                      vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

        // Create descriptor pool
        LavaCake::DescriptorPool descriptorPool = LavaCake::DescriptorPool::Builder(device)
            .setMaxSets(1)
            .addStorageBuffers(3)
            .build();

        // Create descriptor set layout
        LavaCake::DescriptorSetLayout descriptorLayout = LavaCake::DescriptorSetLayout::Builder(device)
            .addStorageBuffer(0, vk::ShaderStageFlagBits::eCompute)
            .addStorageBuffer(1, vk::ShaderStageFlagBits::eCompute)
            .addStorageBuffer(2, vk::ShaderStageFlagBits::eCompute)
            .build();

        // Allocate and update descriptor set
        vk::DescriptorSet descriptorSet = descriptorPool.allocate(descriptorLayout);

        LavaCake::DescriptorSetUpdater(device, descriptorSet)
            .bindStorageBuffer(0, bufferA)
            .bindStorageBuffer(1, bufferB)
            .bindStorageBuffer(2, bufferC)
            .update();

        // Create compute pipeline from shader code
        LavaCake::ComputePipeline pipeline = LavaCake::ComputePipeline::Builder(device)
            .setShaderFromFile(root+"shaders/vectorAddition.comp", LavaCake::ShadingLanguage::eGLSL)
            .addDescriptorSetLayout(descriptorLayout)
            .build();

        // Record command buffer
        vk::CommandBuffer cmd = device.allocateCommandBuffer();
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        cmd.begin(beginInfo);

        // Bind pipeline and descriptor set
        pipeline.bind(cmd);
        pipeline.bindDescriptorSets(cmd, {descriptorSet});

        // Dispatch compute work (size work items, 256 per workgroup)
        pipeline.dispatch(cmd, size);

        cmd.end();

        // Submit and wait
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        auto queue = device.getAnyQueue();
        queue.submit(submitInfo);
        queue.waitIdle();

        device.freeCommandBuffer(cmd);

        // Read results back to CPU
        void* data = bufferC.map();
        std::vector<float> c(size);
        memcpy(c.data(), data, size * sizeof(float));
        bufferC.unmap();

        // Verify results
        std::cout << "Verifying results...\n";
        bool success = true;
        for (uint32_t i = 0; i < size; i++) {
            float expected = a[i] + b[i];
            if (std::abs(c[i] - expected) > 0.0001f) {
                std::cerr << "Error at index " << i << ": expected " << expected
                         << ", got " << c[i] << "\n";
                success = false;
                break;
            }
        }

        if (success) {
            std::cout << "Success! All " << size << " results are correct.\n";
            std::cout << "C[0]=" << c[0] << " (expected " << (a[0] + b[0]) << ")\n";
            std::cout << "C[" << size-1 << "]=" << c[size-1]
                     << " (expected " << (a[size-1] + b[size-1]) << ")\n";
        }

        return success ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
