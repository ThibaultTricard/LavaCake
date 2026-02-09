#pragma once

#include "Device.hpp"
#include "Buffer.hpp"
#include "CommandBuffer.hpp"

namespace LavaCake {

    /**
     * \brief Base class for Vulkan acceleration structures
     *
     * This abstract base class provides common functionality for both
     * Bottom-Level (BLAS) and Top-Level (TLAS) acceleration structures.
     */
    class AccelerationStructure {
    public:
        /**
         * \brief Default constructor
         */
        AccelerationStructure() = default;

        /**
         * \brief Deleted copy constructor to prevent GPU pointer duplication
         */
        AccelerationStructure(const AccelerationStructure&) = delete;

        /**
         * \brief Deleted copy assignment to prevent GPU pointer duplication
         */
        AccelerationStructure& operator=(const AccelerationStructure&) = delete;

        /**
         * \brief Move constructor
         */
        AccelerationStructure(AccelerationStructure&& other) noexcept
            : m_accelerationStructure(std::exchange(other.m_accelerationStructure, VK_NULL_HANDLE))
            , m_buffer(std::exchange(other.m_buffer, VK_NULL_HANDLE))
            , m_allocation(std::exchange(other.m_allocation, {}))
            , m_deviceAddress(std::exchange(other.m_deviceAddress, 0))
            , m_device(other.m_device)
            , m_allocator(other.m_allocator)
        {}

        /**
         * \brief Move assignment operator
         */
        AccelerationStructure& operator=(AccelerationStructure&& other) noexcept {
            if (this != &other) {
                cleanup();
                m_accelerationStructure = std::exchange(other.m_accelerationStructure, VK_NULL_HANDLE);
                m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
                m_allocation = std::exchange(other.m_allocation, {});
                m_deviceAddress = std::exchange(other.m_deviceAddress, 0);
                m_device = other.m_device;
                m_allocator = other.m_allocator;
            }
            return *this;
        }

        /**
         * \brief Virtual destructor - cleans up acceleration structure and buffer
         */
        virtual ~AccelerationStructure() {
            cleanup();
        }

        /**
         * \brief Get the Vulkan acceleration structure handle
         * \return the vk::AccelerationStructureKHR handle
         */
        vk::AccelerationStructureKHR getAccelerationStructure() const {
            return m_accelerationStructure;
        }

        /**
         * \brief Get the device address of the acceleration structure
         * \return the device address for use in shaders or TLAS instances
         */
        vk::DeviceAddress getDeviceAddress() const {
            return m_deviceAddress;
        }

        /**
         * \brief Get the backing buffer
         * \return the vk::Buffer handle
         */
        vk::Buffer getBuffer() const {
            return m_buffer;
        }

        /**
         * \brief Implicit conversion to vk::AccelerationStructureKHR
         * \return the vk::AccelerationStructureKHR handle
         */
        operator vk::AccelerationStructureKHR() const {
            return m_accelerationStructure;
        }

    protected:
        vk::AccelerationStructureKHR m_accelerationStructure = VK_NULL_HANDLE;
        vk::Buffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation{};
        vk::DeviceAddress m_deviceAddress = 0;
        vk::Device m_device;
        VmaAllocator m_allocator = VK_NULL_HANDLE;

        /**
         * \brief Cleans up resources
         */
        void cleanup() {
            if (m_accelerationStructure && m_device) {
                m_device.destroyAccelerationStructureKHR(m_accelerationStructure);
                m_accelerationStructure = VK_NULL_HANDLE;
            }
            if (m_buffer && m_allocator) {
                vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
                m_buffer = VK_NULL_HANDLE;
            }
        }

        /**
         * \brief Creates a scratch buffer for building acceleration structures
         * \param size the required scratch buffer size
         * \param scratchBuffer output parameter for the created buffer
         * \param scratchAllocation output parameter for the VMA allocation
         * \return the device address of the scratch buffer
         */
        vk::DeviceAddress createScratchBuffer(vk::DeviceSize size,
                                               VkBuffer& scratchBuffer,
                                               VmaAllocation& scratchAllocation) {
            vk::BufferCreateInfo scratchBufferInfo{};
            scratchBufferInfo.size = size;
            scratchBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                      vk::BufferUsageFlagBits::eShaderDeviceAddress;

            VmaAllocationCreateInfo scratchAllocInfo{};
            scratchAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            scratchAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            if (vmaCreateBuffer(
                    m_allocator,
                    reinterpret_cast<VkBufferCreateInfo*>(&scratchBufferInfo),
                    &scratchAllocInfo,
                    &scratchBuffer,
                    &scratchAllocation,
                    nullptr) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create scratch buffer");
            }

            vk::BufferDeviceAddressInfo scratchAddressInfo{};
            scratchAddressInfo.buffer = vk::Buffer(scratchBuffer);
            return m_device.getBufferAddress(scratchAddressInfo);
        }

        /**
         * \brief Creates the backing buffer for the acceleration structure
         * \param size the required buffer size
         */
        void createASBuffer(vk::DeviceSize size) {
            vk::BufferCreateInfo bufferInfo{};
            bufferInfo.size = size;
            bufferInfo.usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                               vk::BufferUsageFlagBits::eShaderDeviceAddress;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            VkBuffer buffer;
            if (vmaCreateBuffer(
                    m_allocator,
                    reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
                    &allocInfo,
                    &buffer,
                    &m_allocation,
                    nullptr) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create acceleration structure buffer");
            }
            m_buffer = vk::Buffer(buffer);
        }
    };

    // ---------------------------------------------------------------
    // Bottom-Level Acceleration Structure (BLAS)
    // ---------------------------------------------------------------

    /**
     * \brief Bottom-Level Acceleration Structure for geometry
     *
     * BLAS contains the actual geometry (triangles, AABBs, etc.) that will be
     * ray traced against. Multiple BLAS can be instanced in a TLAS.
     *
     * Example usage:
     * \code
     * vk::AccelerationStructureGeometryKHR geometry{};
     * geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
     * geometry.geometry.triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
     * geometry.geometry.triangles.vertexData.deviceAddress = vertexBufferAddress;
     * // ... set other fields
     *
     * auto blas = BottomLevelAccelerationStructure::Builder(device)
     *     .addGeometry(geometry, triangleCount)
     *     .setPreferFastTrace(true)
     *     .build();
     * \endcode
     */
    class BottomLevelAccelerationStructure : public AccelerationStructure {
    public:
        class Builder;

        BottomLevelAccelerationStructure() = default;

        BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& other) noexcept
            : AccelerationStructure(std::move(other))
        {}

        BottomLevelAccelerationStructure& operator=(BottomLevelAccelerationStructure&& other) noexcept {
            AccelerationStructure::operator=(std::move(other));
            return *this;
        }

        /**
         * \brief Builder for creating Bottom-Level Acceleration Structures
         */
        class Builder {
        public:
            /**
             * \brief Constructs a Builder for BLAS
             * \param device the LavaCake device
             */
            explicit Builder(const LavaCake::Device& device)
                : m_device(device)
                , m_vkDevice(device.getDevice())
                , m_allocator(device.getAllocator())
            {}

            /**
             * \brief Adds geometry to the BLAS
             * \param geometry the acceleration structure geometry description
             * \param primitiveCount number of primitives (triangles for triangle geometry)
             * \param primitiveOffset byte offset into the primitive data (default: 0)
             * \return reference to this builder for method chaining
             */
            Builder& addGeometry(const vk::AccelerationStructureGeometryKHR& geometry,
                                uint32_t primitiveCount,
                                uint32_t primitiveOffset = 0) {
                m_geometries.push_back(geometry);
                m_primitiveCounts.push_back(primitiveCount);

                vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{};
                rangeInfo.primitiveCount = primitiveCount;
                rangeInfo.primitiveOffset = primitiveOffset;
                rangeInfo.firstVertex = 0;
                rangeInfo.transformOffset = 0;
                m_buildRanges.push_back(rangeInfo);

                return *this;
            }

            /**
             * \brief Adds a new triangle geometry with default values
             * \return reference to this builder for method chaining
             *
             * After calling this, use the setter methods to configure the geometry:
             * - setVertexBuffer() to set vertex data
             * - setIndexBuffer() to set index data (optional)
             * - setPrimitiveCount() to set the number of triangles
             * - setOpaque() to set opacity flag
             */
            Builder& addTriangleGeometry() {
                vk::AccelerationStructureGeometryKHR geometry{};
                geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
                geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
                geometry.geometry.triangles.sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
                geometry.geometry.triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
                geometry.geometry.triangles.indexType = vk::IndexType::eNoneKHR;

                m_geometries.push_back(geometry);
                m_primitiveCounts.push_back(0);

                vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{};
                m_buildRanges.push_back(rangeInfo);

                return *this;
            }

            /**
             * \brief Adds a new AABB geometry with default values
             * \return reference to this builder for method chaining
             *
             * After calling this, use the setter methods to configure the geometry:
             * - setAABBBuffer() to set AABB data
             * - setPrimitiveCount() to set the number of AABBs
             * - setOpaque() to set opacity flag
             */
            Builder& addAABBGeometry() {
                vk::AccelerationStructureGeometryKHR geometry{};
                geometry.geometryType = vk::GeometryTypeKHR::eAabbs;
                geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
                geometry.geometry.aabbs.sType = vk::StructureType::eAccelerationStructureGeometryAabbsDataKHR;
                geometry.geometry.aabbs.stride = 24; // sizeof(VkAabbPositionsKHR)

                m_geometries.push_back(geometry);
                m_primitiveCounts.push_back(0);

                vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{};
                m_buildRanges.push_back(rangeInfo);

                return *this;
            }

            /**
             * \brief Sets the vertex buffer for the last added triangle geometry
             * \param address device address of the vertex buffer
             * \param stride stride between vertices in bytes
             * \param vertexCount number of vertices
             * \param format format of vertex positions (default: eR32G32B32Sfloat)
             * \return reference to this builder for method chaining
             */
            Builder& setVertexBuffer(vk::DeviceAddress address,
                                     vk::DeviceSize stride,
                                     uint32_t vertexCount,
                                     vk::Format format = vk::Format::eR32G32B32Sfloat) {
                auto& geometry = m_geometries.back();
                geometry.geometry.triangles.vertexData.deviceAddress = address;
                geometry.geometry.triangles.vertexStride = stride;
                geometry.geometry.triangles.maxVertex = vertexCount - 1;
                geometry.geometry.triangles.vertexFormat = format;
                return *this;
            }

            /**
             * \brief Sets the index buffer for the last added triangle geometry
             * \param address device address of the index buffer
             * \param indexCount number of indices
             * \param indexType type of indices (default: eUint32)
             * \return reference to this builder for method chaining
             */
            Builder& setIndexBuffer(vk::DeviceAddress address,
                                    uint32_t indexCount,
                                    vk::IndexType indexType = vk::IndexType::eUint32) {
                auto& geometry = m_geometries.back();
                geometry.geometry.triangles.indexData.deviceAddress = address;
                geometry.geometry.triangles.indexType = indexType;

                // Update primitive count based on indices
                m_primitiveCounts.back() = indexCount / 3;
                m_buildRanges.back().primitiveCount = indexCount / 3;
                return *this;
            }

            /**
             * \brief Sets the AABB buffer for the last added AABB geometry
             * \param address device address of the AABB buffer
             * \param aabbCount number of AABBs
             * \param stride stride between AABBs (default: 24 bytes)
             * \return reference to this builder for method chaining
             */
            Builder& setAABBBuffer(vk::DeviceAddress address,
                                   uint32_t aabbCount,
                                   vk::DeviceSize stride = 24) {
                auto& geometry = m_geometries.back();
                geometry.geometry.aabbs.data.deviceAddress = address;
                geometry.geometry.aabbs.stride = stride;

                m_primitiveCounts.back() = aabbCount;
                m_buildRanges.back().primitiveCount = aabbCount;
                return *this;
            }

            /**
             * \brief Sets the primitive count for the last added geometry
             * \param count number of primitives (triangles or AABBs)
             * \return reference to this builder for method chaining
             */
            Builder& setPrimitiveCount(uint32_t count) {
                m_primitiveCounts.back() = count;
                m_buildRanges.back().primitiveCount = count;
                return *this;
            }

            /**
             * \brief Sets the primitive offset for the last added geometry
             * \param offset byte offset into the primitive data
             * \return reference to this builder for method chaining
             */
            Builder& setPrimitiveOffset(uint32_t offset) {
                m_buildRanges.back().primitiveOffset = offset;
                return *this;
            }

            /**
             * \brief Sets whether the last added geometry is opaque
             * \param opaque true for opaque geometry (default), false for transparent
             * \return reference to this builder for method chaining
             */
            Builder& setOpaque(bool opaque = true) {
                auto& geometry = m_geometries.back();
                if (opaque) {
                    geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
                } else {
                    geometry.flags = vk::GeometryFlagsKHR{};
                }
                return *this;
            }

            /**
             * \brief Enables update support for the acceleration structure
             * \param allow whether to allow updates (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setAllowUpdate(bool allow = true) {
                if (allow) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
                } else {
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
                }
                return *this;
            }

            /**
             * \brief Prefers fast ray tracing performance over build time
             * \param prefer whether to prefer fast trace (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setPreferFastTrace(bool prefer = true) {
                if (prefer) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
                }
                return *this;
            }

            /**
             * \brief Prefers fast build time over ray tracing performance
             * \param prefer whether to prefer fast build (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setPreferFastBuild(bool prefer = true) {
                if (prefer) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
                }
                return *this;
            }

            /**
             * \brief Allows compaction of the acceleration structure
             * \param allow whether to allow compaction (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setAllowCompaction(bool allow = true) {
                if (allow) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
                } else {
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
                }
                return *this;
            }

            /**
             * \brief Builds the Bottom-Level Acceleration Structure
             * \return the constructed BottomLevelAccelerationStructure
             * \throws std::runtime_error if building fails
             */
            BottomLevelAccelerationStructure build() {
                if (m_geometries.empty()) {
                    throw std::runtime_error("BLAS requires at least one geometry");
                }

                BottomLevelAccelerationStructure blas;
                blas.m_device = m_vkDevice;
                blas.m_allocator = m_allocator;

                // Prepare build geometry info
                vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
                buildInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
                buildInfo.flags = m_buildFlags;
                buildInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
                buildInfo.geometryCount = static_cast<uint32_t>(m_geometries.size());
                buildInfo.pGeometries = m_geometries.data();

                // Query size requirements
                vk::AccelerationStructureBuildSizesInfoKHR sizeInfo =
                    m_vkDevice.getAccelerationStructureBuildSizesKHR(
                        vk::AccelerationStructureBuildTypeKHR::eDevice,
                        buildInfo,
                        m_primitiveCounts);

                // Create AS buffer
                blas.createASBuffer(sizeInfo.accelerationStructureSize);

                // Create acceleration structure
                vk::AccelerationStructureCreateInfoKHR createInfo{};
                createInfo.buffer = blas.m_buffer;
                createInfo.size = sizeInfo.accelerationStructureSize;
                createInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;

                blas.m_accelerationStructure = m_vkDevice.createAccelerationStructureKHR(createInfo);

                // Get device address
                vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{};
                addressInfo.accelerationStructure = blas.m_accelerationStructure;
                blas.m_deviceAddress = m_vkDevice.getAccelerationStructureAddressKHR(addressInfo);

                // Create scratch buffer
                VkBuffer scratchBuffer;
                VmaAllocation scratchAllocation;
                vk::DeviceAddress scratchAddress = blas.createScratchBuffer(
                    sizeInfo.buildScratchSize, scratchBuffer, scratchAllocation);

                // Build the acceleration structure
                buildInfo.dstAccelerationStructure = blas.m_accelerationStructure;
                buildInfo.scratchData.deviceAddress = scratchAddress;

                // Record and submit build commands
                CommandBuffer cmd(m_device);
                cmd.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

                std::vector<const vk::AccelerationStructureBuildRangeInfoKHR*> buildRangePtrs;
                for (auto& range : m_buildRanges) {
                    buildRangePtrs.push_back(&range);
                }

                cmd.getCommandBuffer().buildAccelerationStructuresKHR(buildInfo, buildRangePtrs);

                cmd.end();

                vk::SubmitInfo submitInfo{};
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = cmd;
                m_device.getAnyQueue().submit(submitInfo);
                m_device.getAnyQueue().waitIdle();

                // Clean up scratch buffer
                vmaDestroyBuffer(m_allocator, scratchBuffer, scratchAllocation);

                return blas;
            }

        private:
            const LavaCake::Device& m_device;
            vk::Device m_vkDevice;
            VmaAllocator m_allocator;

            std::vector<vk::AccelerationStructureGeometryKHR> m_geometries;
            std::vector<vk::AccelerationStructureBuildRangeInfoKHR> m_buildRanges;
            std::vector<uint32_t> m_primitiveCounts;

            vk::BuildAccelerationStructureFlagsKHR m_buildFlags =
                vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
        };
    };

    // ---------------------------------------------------------------
    // Top-Level Acceleration Structure (TLAS)
    // ---------------------------------------------------------------

    /**
     * \brief Top-Level Acceleration Structure for instances
     *
     * TLAS contains instances of BLAS with their transforms. This is what
     * shaders trace rays against to find intersections.
     *
     * Example usage:
     * \code
     * auto tlas = TopLevelAccelerationStructure::Builder(device)
     *     .setInstances(instanceBufferAddress, instanceCount)
     *     .build();
     * \endcode
     */
    class TopLevelAccelerationStructure : public AccelerationStructure {
    public:
        class Builder;

        TopLevelAccelerationStructure() = default;

        TopLevelAccelerationStructure(TopLevelAccelerationStructure&& other) noexcept
            : AccelerationStructure(std::move(other))
        {}

        TopLevelAccelerationStructure& operator=(TopLevelAccelerationStructure&& other) noexcept {
            AccelerationStructure::operator=(std::move(other));
            return *this;
        }

        /**
         * \brief Builder for creating Top-Level Acceleration Structures
         */
        class Builder {
        public:
            /**
             * \brief Constructs a Builder for TLAS
             * \param device the LavaCake device
             */
            explicit Builder(const LavaCake::Device& device)
                : m_device(device)
                , m_vkDevice(device.getDevice())
                , m_allocator(device.getAllocator())
            {}

            /**
             * \brief Sets the instance buffer for the TLAS
             * \param instanceBufferAddress device address of the instance buffer
             * \param instanceCount number of instances
             * \return reference to this builder for method chaining
             */
            Builder& setInstances(vk::DeviceAddress instanceBufferAddress, uint32_t instanceCount) {
                m_instanceBufferAddress = instanceBufferAddress;
                m_instanceCount = instanceCount;
                return *this;
            }

            /**
             * \brief Enables update support for the acceleration structure
             * \param allow whether to allow updates (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setAllowUpdate(bool allow = true) {
                if (allow) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
                } else {
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
                }
                return *this;
            }

            /**
             * \brief Prefers fast ray tracing performance over build time
             * \param prefer whether to prefer fast trace (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setPreferFastTrace(bool prefer = true) {
                if (prefer) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
                }
                return *this;
            }

            /**
             * \brief Prefers fast build time over ray tracing performance
             * \param prefer whether to prefer fast build (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setPreferFastBuild(bool prefer = true) {
                if (prefer) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
                }
                return *this;
            }

            /**
             * \brief Allows compaction of the acceleration structure
             * \param allow whether to allow compaction (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setAllowCompaction(bool allow = true) {
                if (allow) {
                    m_buildFlags |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
                } else {
                    m_buildFlags &= ~vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
                }
                return *this;
            }

            /**
             * \brief Builds the Top-Level Acceleration Structure
             * \return the constructed TopLevelAccelerationStructure
             * \throws std::runtime_error if building fails
             */
            TopLevelAccelerationStructure build() {
                if (m_instanceBufferAddress == 0 || m_instanceCount == 0) {
                    throw std::runtime_error("TLAS requires instance buffer and count");
                }

                TopLevelAccelerationStructure tlas;
                tlas.m_device = m_vkDevice;
                tlas.m_allocator = m_allocator;

                // Setup instance geometry
                vk::AccelerationStructureGeometryKHR instanceGeometry{};
                instanceGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
                instanceGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
                instanceGeometry.geometry.instances.sType =
                    vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;
                instanceGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
                instanceGeometry.geometry.instances.data.deviceAddress = m_instanceBufferAddress;

                // Prepare build geometry info
                vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
                buildInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
                buildInfo.flags = m_buildFlags;
                buildInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
                buildInfo.geometryCount = 1;
                buildInfo.pGeometries = &instanceGeometry;

                // Query size requirements
                std::vector<uint32_t> primitiveCounts = { m_instanceCount };
                vk::AccelerationStructureBuildSizesInfoKHR sizeInfo =
                    m_vkDevice.getAccelerationStructureBuildSizesKHR(
                        vk::AccelerationStructureBuildTypeKHR::eDevice,
                        buildInfo,
                        primitiveCounts);

                // Create AS buffer
                tlas.createASBuffer(sizeInfo.accelerationStructureSize);

                // Create acceleration structure
                vk::AccelerationStructureCreateInfoKHR createInfo{};
                createInfo.buffer = tlas.m_buffer;
                createInfo.size = sizeInfo.accelerationStructureSize;
                createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;

                tlas.m_accelerationStructure = m_vkDevice.createAccelerationStructureKHR(createInfo);

                // Get device address
                vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{};
                addressInfo.accelerationStructure = tlas.m_accelerationStructure;
                tlas.m_deviceAddress = m_vkDevice.getAccelerationStructureAddressKHR(addressInfo);

                // Create scratch buffer
                VkBuffer scratchBuffer;
                VmaAllocation scratchAllocation;
                vk::DeviceAddress scratchAddress = tlas.createScratchBuffer(
                    sizeInfo.buildScratchSize, scratchBuffer, scratchAllocation);

                // Build the acceleration structure
                buildInfo.dstAccelerationStructure = tlas.m_accelerationStructure;
                buildInfo.scratchData.deviceAddress = scratchAddress;

                // Build range info
                vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{};
                rangeInfo.primitiveCount = m_instanceCount;
                rangeInfo.primitiveOffset = 0;
                rangeInfo.firstVertex = 0;
                rangeInfo.transformOffset = 0;

                std::vector<const vk::AccelerationStructureBuildRangeInfoKHR*> buildRangePtrs = { &rangeInfo };

                // Record and submit build commands
                CommandBuffer cmd(m_device);
                cmd.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

                cmd.getCommandBuffer().buildAccelerationStructuresKHR(buildInfo, buildRangePtrs);

                cmd.end();

                vk::SubmitInfo submitInfo{};
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = cmd;
                m_device.getAnyQueue().submit(submitInfo);
                m_device.getAnyQueue().waitIdle();

                // Clean up scratch buffer
                vmaDestroyBuffer(m_allocator, scratchBuffer, scratchAllocation);

                return tlas;
            }

        private:
            const LavaCake::Device& m_device;
            vk::Device m_vkDevice;
            VmaAllocator m_allocator;

            vk::DeviceAddress m_instanceBufferAddress = 0;
            uint32_t m_instanceCount = 0;

            vk::BuildAccelerationStructureFlagsKHR m_buildFlags =
                vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
        };
    };

} // namespace LavaCake
