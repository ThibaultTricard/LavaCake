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


