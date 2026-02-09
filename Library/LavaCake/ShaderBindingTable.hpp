#pragma once

#include "Device.hpp"
#include "Buffer.hpp"
#include "RayTracingPipeline.hpp"

namespace LavaCake {

    /**
     * \brief Manages the Shader Binding Table (SBT) for ray tracing
     *
     * The SBT contains shader group handles organized into four regions:
     * - Raygen: Ray generation shaders
     * - Miss: Miss shaders (called when rays don't hit geometry)
     * - Hit: Hit groups (closest-hit, any-hit, intersection shaders)
     * - Callable: Callable shaders
     *
     * Example usage:
     * \code
     * // Create pipeline first
     * auto pipeline = RayTracingPipeline::Builder(device)
     *     .addRaygenShaderFromFile("raygen.rgen")
     *     .addMissShaderFromFile("miss.rmiss")
     *     .beginHitGroup()
     *         .setClosestHitShaderFromFile("hit.rchit")
     *     .endHitGroup()
     *     .build();
     *
     * // Create SBT from pipeline
     * ShaderBindingTable sbt(device, pipeline);
     *
     * // Use in traceRays
     * pipeline.traceRays(cmd,
     *     sbt.getRaygenRegion(),
     *     sbt.getMissRegion(),
     *     sbt.getHitRegion(),
     *     sbt.getCallableRegion(),
     *     width, height, 1);
     * \endcode
     */
    class ShaderBindingTable {
    public:
        /**
         * \brief Default constructor
         */
        ShaderBindingTable() = default;

        /**
         * \brief Deleted copy constructor to prevent GPU pointer duplication
         */
        ShaderBindingTable(const ShaderBindingTable&) = delete;

        /**
         * \brief Deleted copy assignment to prevent GPU pointer duplication
         */
        ShaderBindingTable& operator=(const ShaderBindingTable&) = delete;

        /**
         * \brief Move constructor
         */
        ShaderBindingTable(ShaderBindingTable&& other) noexcept
            : m_buffer(std::move(other.m_buffer))
            , m_raygenRegion(other.m_raygenRegion)
            , m_missRegion(other.m_missRegion)
            , m_hitRegion(other.m_hitRegion)
            , m_callableRegion(other.m_callableRegion)
        {
            other.m_raygenRegion = vk::StridedDeviceAddressRegionKHR{};
            other.m_missRegion = vk::StridedDeviceAddressRegionKHR{};
            other.m_hitRegion = vk::StridedDeviceAddressRegionKHR{};
            other.m_callableRegion = vk::StridedDeviceAddressRegionKHR{};
        }

        /**
         * \brief Move assignment operator
         */
        ShaderBindingTable& operator=(ShaderBindingTable&& other) noexcept {
            if (this != &other) {
                m_buffer = std::move(other.m_buffer);
                m_raygenRegion = other.m_raygenRegion;
                m_missRegion = other.m_missRegion;
                m_hitRegion = other.m_hitRegion;
                m_callableRegion = other.m_callableRegion;
                other.m_raygenRegion = vk::StridedDeviceAddressRegionKHR{};
                other.m_missRegion = vk::StridedDeviceAddressRegionKHR{};
                other.m_hitRegion = vk::StridedDeviceAddressRegionKHR{};
                other.m_callableRegion = vk::StridedDeviceAddressRegionKHR{};
            }
            return *this;
        }

        /**
         * \brief Default destructor
         */
        ~ShaderBindingTable() = default;

        /**
         * \brief Creates a Shader Binding Table from a ray tracing pipeline
         * \param device the LavaCake device
         * \param pipeline the ray tracing pipeline to create the SBT from
         *
         * \details This constructor:
         * 1. Queries RT pipeline properties for handle sizes/alignment
         * 2. Gets shader group handles from the pipeline
         * 3. Calculates properly aligned region sizes
         * 4. Creates a buffer with proper alignment
         * 5. Copies handles to the buffer with correct spacing
         * 6. Sets up the strided device address regions for traceRays
         */
        ShaderBindingTable(const LavaCake::Device& device, const RayTracingPipeline& pipeline) {
            // Get RT pipeline properties
            auto rtProps = device.getRayTracingPipelineProperties();

            uint32_t handleSize = rtProps.shaderGroupHandleSize;
            uint32_t handleAlignment = rtProps.shaderGroupHandleAlignment;
            uint32_t baseAlignment = rtProps.shaderGroupBaseAlignment;

            // Align handle size
            uint32_t handleSizeAligned = alignUp(handleSize, handleAlignment);

            // Get shader group handles from pipeline
            std::vector<uint8_t> handles = pipeline.getShaderGroupHandles();

            // Get counts
            uint32_t raygenCount = pipeline.getRaygenShaderCount();
            uint32_t missCount = pipeline.getMissShaderCount();
            uint32_t hitCount = pipeline.getHitGroupCount();
            uint32_t callableCount = pipeline.getCallableShaderCount();

            // Calculate region sizes (each region must be aligned to baseAlignment)
            vk::DeviceSize raygenSize = alignUp(handleSizeAligned * raygenCount, baseAlignment);
            vk::DeviceSize missSize = alignUp(handleSizeAligned * missCount, baseAlignment);
            vk::DeviceSize hitSize = alignUp(handleSizeAligned * hitCount, baseAlignment);
            vk::DeviceSize callableSize = alignUp(handleSizeAligned * callableCount, baseAlignment);

            vk::DeviceSize totalSize = raygenSize + missSize + hitSize + callableSize;

            if (totalSize == 0) {
                throw std::runtime_error("Shader binding table has no entries");
            }

            // Create SBT buffer
            m_buffer = std::make_unique<Buffer>(device, totalSize,
                vk::BufferUsageFlagBits::eShaderBindingTableKHR |
                vk::BufferUsageFlagBits::eShaderDeviceAddress |
                vk::BufferUsageFlagBits::eTransferDst,
                vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

            // Map and copy handles to buffer
            uint8_t* mapped = static_cast<uint8_t*>(m_buffer->map());

            // Zero out the buffer first (for proper alignment padding)
            std::memset(mapped, 0, totalSize);

            uint32_t handleIdx = 0;
            vk::DeviceSize offset = 0;

            // Copy raygen handles
            for (uint32_t i = 0; i < raygenCount; i++) {
                std::memcpy(mapped + offset + i * handleSizeAligned,
                           handles.data() + handleIdx * handleSize,
                           handleSize);
                handleIdx++;
            }
            offset += raygenSize;

            // Copy miss handles
            for (uint32_t i = 0; i < missCount; i++) {
                std::memcpy(mapped + offset + i * handleSizeAligned,
                           handles.data() + handleIdx * handleSize,
                           handleSize);
                handleIdx++;
            }
            offset += missSize;

            // Copy hit group handles
            for (uint32_t i = 0; i < hitCount; i++) {
                std::memcpy(mapped + offset + i * handleSizeAligned,
                           handles.data() + handleIdx * handleSize,
                           handleSize);
                handleIdx++;
            }
            offset += hitSize;

            // Copy callable handles
            for (uint32_t i = 0; i < callableCount; i++) {
                std::memcpy(mapped + offset + i * handleSizeAligned,
                           handles.data() + handleIdx * handleSize,
                           handleSize);
                handleIdx++;
            }

            m_buffer->unmap();

            // Get buffer device address
            vk::BufferDeviceAddressInfo addressInfo{};
            addressInfo.buffer = *m_buffer;
            vk::DeviceAddress bufferAddress = device.getDevice().getBufferAddress(addressInfo);

            // Set up regions
            vk::DeviceSize regionOffset = 0;

            // Raygen region (spec requires size == stride for raygen)
            m_raygenRegion.deviceAddress = bufferAddress + regionOffset;
            m_raygenRegion.stride = handleSizeAligned;
            m_raygenRegion.size = handleSizeAligned;  // Must equal stride per Vulkan spec
            regionOffset += raygenSize;

            // Miss region
            if (missCount > 0) {
                m_missRegion.deviceAddress = bufferAddress + regionOffset;
                m_missRegion.stride = handleSizeAligned;
                m_missRegion.size = missSize;
            }
            regionOffset += missSize;

            // Hit region
            if (hitCount > 0) {
                m_hitRegion.deviceAddress = bufferAddress + regionOffset;
                m_hitRegion.stride = handleSizeAligned;
                m_hitRegion.size = hitSize;
            }
            regionOffset += hitSize;

            // Callable region
            if (callableCount > 0) {
                m_callableRegion.deviceAddress = bufferAddress + regionOffset;
                m_callableRegion.stride = handleSizeAligned;
                m_callableRegion.size = callableSize;
            }
        }

        /**
         * \brief Get the raygen region for traceRays
         * \return the raygen strided device address region
         */
        const vk::StridedDeviceAddressRegionKHR& getRaygenRegion() const {
            return m_raygenRegion;
        }

        /**
         * \brief Get the miss region for traceRays
         * \return the miss strided device address region
         */
        const vk::StridedDeviceAddressRegionKHR& getMissRegion() const {
            return m_missRegion;
        }

        /**
         * \brief Get the hit region for traceRays
         * \return the hit strided device address region
         */
        const vk::StridedDeviceAddressRegionKHR& getHitRegion() const {
            return m_hitRegion;
        }

        /**
         * \brief Get the callable region for traceRays
         * \return the callable strided device address region
         */
        const vk::StridedDeviceAddressRegionKHR& getCallableRegion() const {
            return m_callableRegion;
        }

        /**
         * \brief Get the underlying SBT buffer
         * \return reference to the buffer
         */
        const Buffer& getBuffer() const {
            return *m_buffer;
        }

    private:
        /**
         * \brief Aligns a value up to the specified alignment
         * \param value the value to align
         * \param alignment the alignment (must be power of 2)
         * \return the aligned value
         */
        static vk::DeviceSize alignUp(vk::DeviceSize value, vk::DeviceSize alignment) {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        std::unique_ptr<Buffer> m_buffer;

        vk::StridedDeviceAddressRegionKHR m_raygenRegion{};
        vk::StridedDeviceAddressRegionKHR m_missRegion{};
        vk::StridedDeviceAddressRegionKHR m_hitRegion{};
        vk::StridedDeviceAddressRegionKHR m_callableRegion{};
    };

} // namespace LavaCake
