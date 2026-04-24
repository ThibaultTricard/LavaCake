#pragma once
#include "Device.hpp"

namespace LavaCake
{
    /**
     * \brief Manages a Vulkan command buffer with optional fence synchronization
     *
     * This class wraps a command buffer and provides automatic fence management
     * for tracking GPU execution completion.
     *
     * Example usage:
     * \code
     * CommandBuffer cmd(device, true); // Create with fence
     * cmd.begin();
     * // Record commands...
     * cmd.end();
     *
     * // Manual submission with fence
     * vk::SubmitInfo submitInfo{};
     * submitInfo.commandBufferCount = 1;
     * submitInfo.pCommandBuffers = &cmd.getCommandBuffer();
     * queue.submit(submitInfo, cmd.getFence());
     * cmd.markSubmitted();
     *
     * cmd.waitForCompletion();
     * \endcode
     */
    class CommandBuffer {
    public:
        /**
         * \brief Default constructor
         */
        CommandBuffer() = default;

        /**
         * \brief Deleted copy constructor to prevent fence duplication
         */
        CommandBuffer(const CommandBuffer&) = delete;

        /**
         * \brief Deleted copy assignment to prevent fence duplication
         */
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        /**
         * \brief Move constructor
         */
        CommandBuffer(CommandBuffer&& other) noexcept
            : m_cmd(other.m_cmd)
            , m_fence(other.m_fence)
            , m_device(other.m_device)
            , m_commandPool(other.m_commandPool)
            , m_submitted(other.m_submitted)
            , m_hasFence(other.m_hasFence)
        {
            other.m_fence = nullptr;
            other.m_cmd = nullptr;
            other.m_hasFence = false;
        }

        /**
         * \brief Move assignment operator
         */
        CommandBuffer& operator=(CommandBuffer&& other) noexcept {
            if (this != &other) {
                cleanup();
                m_cmd = other.m_cmd;
                m_fence = other.m_fence;
                m_device = other.m_device;
                m_commandPool = other.m_commandPool;
                m_submitted = other.m_submitted;
                m_hasFence = other.m_hasFence;
                other.m_fence = nullptr;
                other.m_cmd = nullptr;
                other.m_hasFence = false;
            }
            return *this;
        }

        /**
         * \brief Constructor with device and optional fence
         * \param device the device on which the command buffer will be created
         * \param createFence whether to create a fence for synchronization
         */
        CommandBuffer(const LavaCake::Device& device, bool createFence = false)
            : CommandBuffer(device, device.getCommandPool(), createFence) { }


        /**
         * \brief Constructor with device and optional fence
         * \param device the device on which the command buffer will be created
         * \param commandPool the commandpool used to create the command Buffer
         * \param createFence whether to create a fence for synchronization
         */
        CommandBuffer(const vk::Device& device, const vk::CommandPool commandPool, bool createFence = false)
            : m_device(device)
            , m_commandPool(commandPool)
            , m_hasFence(createFence)
        {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_commandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = 1;
            m_cmd = m_device.allocateCommandBuffers(allocInfo)[0];

            if (createFence) {
                vk::FenceCreateInfo fenceInfo{};
                m_fence = m_device.createFence(fenceInfo);
            }
        }


        /**
         * \brief Begin recording commands
         * \param flags command buffer usage flags
         */
        void begin(vk::CommandBufferUsageFlags flags = {}) {
            vk::CommandBufferBeginInfo beginInfo{};
            beginInfo.flags = flags;
            m_cmd.begin(beginInfo);
        }

        /**
         * \brief End recording commands
         */
        void end() {
            m_cmd.end();
        }

        /**
         * \brief Records a pipeline barrier command into the command buffer.
         *
         * A pipeline barrier defines an execution and memory dependency between
         * commands submitted before and after the barrier. It can synchronize
         * access to buffers and images as well as perform image layout transitions.
         *
         * \param srcStageMask           Pipeline stages that must complete before the barrier.
         * \param dstStageMask           Pipeline stages that are blocked until the barrier is resolved.
         * \param dependencyFlags        Flags specifying how the dependency is formed (e.g., by-region).
         * \param memoryBarriers         Global memory barriers applying to all memory.
         * \param bufferMemoryBarriers   Buffer-specific memory barriers, optionally transferring queue ownership.
         * \param imageMemoryBarriers    Image-specific memory barriers, optionally changing the image layout or transferring queue ownership.
         */
        void pipelineBarrier(
            vk::PipelineStageFlags                        srcStageMask,
            vk::PipelineStageFlags                        dstStageMask,
            vk::DependencyFlags                           dependencyFlags,
            vk::ArrayProxy<const vk::MemoryBarrier>       memoryBarriers,
            vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
            vk::ArrayProxy<const vk::ImageMemoryBarrier>  imageMemoryBarriers
        ) {
            m_cmd.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, memoryBarriers, bufferMemoryBarriers, imageMemoryBarriers);
        }

        void pipelineBarrier2(const vk::DependencyInfo& dependencyInfo) {
            m_cmd.pipelineBarrier2(dependencyInfo);
        }

        void bindDescriptorSets(
            vk::PipelineBindPoint                        pipelineBindPoint,
            vk::PipelineLayout                           layout,
            uint32_t                                     firstSet,
            vk::ArrayProxy<const vk::DescriptorSet>      descriptorSets,
            vk::ArrayProxy<const uint32_t>               dynamicOffsets = {}
        ) {
            m_cmd.bindDescriptorSets(pipelineBindPoint, layout, firstSet, descriptorSets, dynamicOffsets);
        }

        void bindIndexBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::IndexType indexType) {
            m_cmd.bindIndexBuffer(buffer, offset, indexType);
        }

        void bindVertexBuffers(
            uint32_t                                firstBinding,
            vk::ArrayProxy<const vk::Buffer>        buffers,
            vk::ArrayProxy<const vk::DeviceSize>    offsets
        ) {
            m_cmd.bindVertexBuffers(firstBinding, buffers, offsets);
        }

        template<typename T>
        void pushConstants(
            vk::PipelineLayout     layout,
            vk::ShaderStageFlags   stageFlags,
            uint32_t               offset,
            const T&               value
        ) {
            m_cmd.pushConstants(layout, stageFlags, offset, sizeof(T), &value);
        }

        void fillBuffer(vk::Buffer dstBuffer, vk::DeviceSize dstOffset, vk::DeviceSize size, uint32_t data) {
            m_cmd.fillBuffer(dstBuffer, dstOffset, size, data);
        }

        void executeCommands(vk::ArrayProxy<const vk::CommandBuffer> commandBuffers) {
            m_cmd.executeCommands(commandBuffers);
        }

        /**
         * \brief Get the fence for manual submission
         * \return the vk::Fence handle (can be null if no fence was created)
         */
        vk::Fence getFence() const {
            return m_fence;
        }

        /**
         * \brief Mark the command buffer as submitted (for fence tracking)
         * Call this after manually submitting to a queue with the fence
         */
        void markSubmitted() {
            m_submitted = true;
        }

        /**
         * \brief Wait for command buffer execution to complete
         * \param timeout timeout in nanoseconds (default: infinite)
         * \return vk::Result indicating success or timeout
         */
        vk::Result waitForCompletion(uint64_t timeout = UINT64_MAX) {
            if (m_hasFence && m_submitted) {
                return m_device.waitForFences(m_fence, VK_TRUE, timeout);
            }
            return vk::Result::eSuccess;
        }

        /**
         * \brief Check if command buffer execution is complete
         * \return true if complete or no fence, false if still executing
         */
        bool isComplete() const {
            if (!m_hasFence || !m_submitted) {
                return true;
            }
            vk::Result result = m_device.getFenceStatus(m_fence);
            return result == vk::Result::eSuccess;
        }

        /**
         * \brief Reset the command buffer and fence for reuse
         */
        void reset() {
            if (m_hasFence) {
                m_device.resetFences(m_fence);
            }
            m_cmd.reset();
            m_submitted = false;
        }

        /**
         * \brief Get the underlying Vulkan command buffer
         * \return the vk::CommandBuffer handle
         */
        vk::CommandBuffer getCommandBuffer() const {
            return m_cmd;
        }

        /**
         * \brief Implicit conversion to vk::CommandBuffer (const)
         * \return the vk::CommandBuffer handle
         */
        operator vk::CommandBuffer() const {
            return m_cmd;
        }

        /**
         * \brief Implicit conversion to vk::CommandBuffer& (non-const reference)
         * Allows passing CommandBuffer to functions expecting vk::CommandBuffer&
         * \return reference to the vk::CommandBuffer handle
         */
        operator vk::CommandBuffer&() {
            return m_cmd;
        }

        /**
         * \brief Implicit conversion to const vk::CommandBuffer* (const pointer)
         * Allows passing CommandBuffer to functions expecting vk::CommandBuffer*
         * \return reference to the vk::CommandBuffer handle
         */
        operator const vk::CommandBuffer*() const{
            return &m_cmd;
        }


        /**
         * \brief Destructor - cleans up fence and command buffer
         */
        ~CommandBuffer() {
            cleanup();
        }

    private:
        vk::CommandBuffer m_cmd;           ///< The Vulkan command buffer
        vk::Fence m_fence;                 ///< Fence for synchronization
        vk::Device m_device;               ///< Associated device
        vk::CommandPool m_commandPool;     ///< Associated device
        bool m_submitted = false;          ///< Whether the command buffer has been submitted
        bool m_hasFence = false;           ///< Whether a fence was created

        /**
         * \brief Clean up resources
         */
        void cleanup() {
            if (m_hasFence && m_fence) {
                // Wait for any pending operations before destroying
                if (m_submitted) {
                    (void)m_device.waitForFences(m_fence, VK_TRUE, UINT64_MAX);
                }
                m_device.destroyFence(m_fence);
                m_fence = nullptr;
            }
            if (m_cmd) {
                m_device.freeCommandBuffers(m_commandPool, m_cmd);
                m_cmd = nullptr;
            }
        }
    };
} // namespace LavaCake


