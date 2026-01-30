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
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/Buffer.hpp>

#include <LavaCake/ComputePipeline.hpp>
#include <LavaCake/DescriptorPool.hpp>


std::string root = PROJECT_ROOT;


int main() {
    // Create a headless device (no window needed for compute)
    bool success = true;
    LavaCake::Device device = LavaCake::Device::Builder()
                            .setComputeQueueCount(1)
                            .build();
    {

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
            .setShaderFromFile(root+"shaders/vector_addition.comp", LavaCake::ShadingLanguage::eGLSL)
            .addDescriptorSetLayout(descriptorLayout)
            .build();

        // Create command buffer with fence for synchronization
        LavaCake::CommandBuffer cmd(device, true);

        // Record command buffer
        cmd.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        // Bind pipeline and descriptor set
        pipeline.bind(cmd);
        pipeline.bindDescriptorSets(cmd, {descriptorSet});

        // Dispatch compute work (size work items, 256 per workgroup)
        pipeline.dispatch(cmd, size);

        cmd.end();

        // Submit with fence
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        vk::CommandBuffer rawCmd = cmd.getCommandBuffer();
        submitInfo.pCommandBuffers = &rawCmd;

        auto queue = device.getAnyQueue();
        queue.submit(submitInfo, cmd.getFence());
        cmd.markSubmitted();

        // Wait for compute to complete
        cmd.waitForCompletion();

        // Read results back to CPU
        void* data = bufferC.map();
        std::vector<float> c(size);
        memcpy(c.data(), data, size * sizeof(float));
        bufferC.unmap();

        // Verify results
        std::cout << "Verifying results...\n";
        
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
    }

    device.releaseDevice();

    return success ? 0 : 1;

}
