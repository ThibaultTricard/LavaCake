#pragma once
#include "./Buffer.hpp"
#include "./ByteDictionary.hpp"
#include <vector>
#include <map>
#include <span>
#include <iostream>
#include <string>

namespace LavaCake {

    /**
     * \brief Manages uniform buffer creation and GPU memory updates
     *
     * This class is a dictionary of variables of different types that can be sent to the GPU.
     * Variables are stored contiguously and can be updated individually or in batch.
     *
     * Example usage:
     * \code
     * UniformBuffer ubo(device);
     *
     * glm::mat4 modelMatrix = glm::mat4(1.0f);
     * glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
     *
     * ubo.addVariable("model", modelMatrix);
     * ubo.addVariable("color", color);
     * ubo.end();
     *
     * // Later, update variables
     * ubo.setVariable("color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
     * ubo.update(commandBuffer);
     * \endcode
     */
    class UniformBuffer {
    public:
        /**
         * \brief Default constructor
         */
        UniformBuffer() = default;

        /**
         * \brief Destructor
         */
        ~UniformBuffer() = default;

        /**
         * \brief Deleted copy constructor to prevent GPU pointer duplication
         */
        UniformBuffer(const UniformBuffer&) = delete;

        /**
         * \brief Deleted copy assignment to prevent GPU pointer duplication
         */
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        /**
         * \brief Move constructor
         */
        UniformBuffer(UniformBuffer&&) noexcept = default;

        /**
         * \brief Move assignment operator
         */
        UniformBuffer& operator=(UniformBuffer&&) noexcept = default;

        /**
         * \brief Constructor with device
         * \param device the device on which the buffer will be created
         */
        explicit UniformBuffer(const LavaCake::Device& device)
            : m_device(device)
        {}

        /**
         * \brief Add a variable into the dictionary
         * \param name the name of the variable
         * \param value the variable (simple data type)
         */
        template<typename T>
        void addVariable(const std::string& name, const T& value) {
            m_variables.addVariableRange(name, std::span<const T, 1>{ &value, 1 });
        }

        /**
         * \brief Add a variable into the dictionary
         * \param name the name of the variable
         * \param value the variable (vector of data)
         */
        template<typename T>
        void addVariable(const std::string& name, const std::vector<T>& value) {
            m_variables.addVariableRange(name, std::span{ value });
        }

        /**
         * \brief Add a variable into the dictionary
         * \param name the name of the variable
         * \param value the variable (array of data)
         */
        template<typename T, std::size_t N>
        void addVariable(const std::string& name, const std::array<T, N>& value) {
            m_variables.addVariableRange(name, std::span{ value });
        }

        /**
         * \brief Add a variable into the dictionary
         * \param name the name of the variable
         * \param value the variable (span of data)
         */
        template<typename T, std::size_t Extent>
        void addVariable(const std::string& name, const std::span<T, Extent>& value) {
            m_variables.addVariableRange(name, value);
        }

        /**
         * \brief Set a variable in the dictionary
         * \param name the name of the variable
         * \param value the new value (simple data type)
         */
        template<typename T>
        void setVariable(const std::string& name, const T& value) {
            m_variables.setVariableRange(name, std::span<const T, 1>{ &value, 1 });
        }

        /**
         * \brief Set a variable in the dictionary
         * \param name the name of the variable
         * \param value the new value (vector of data)
         */
        template<typename T>
        void setVariable(const std::string& name, const std::vector<T>& value) {
            m_variables.setVariableRange(name, std::span{ value });
        }

        /**
         * \brief Set a variable in the dictionary
         * \param name the name of the variable
         * \param value the new value (array of data)
         */
        template<typename T, std::size_t N>
        void setVariable(const std::string& name, const std::array<T, N>& value) {
            m_variables.setVariableRange(name, std::span{ value });
        }

        /**
         * \brief Set a variable in the dictionary
         * \param name the name of the variable
         * \param value the new value (span of data)
         */
        template<typename T, std::size_t Extent>
        void setVariable(const std::string& name, const std::span<T, Extent>& value) {
            m_variables.setVariableRange(name, value);
        }

        /**
         * \brief Finalize the buffer allocation
         *
         * Call this after adding all variables. This allocates both the staging
         * buffer (host-visible) and the main buffer (device-local).
         */
        void end() {
            VkDeviceSize bufferSize = m_variables.data().size();

            // Allocate staging buffer (host-visible for CPU writes)
            m_stagingBuffer = Buffer(
                m_device,
                bufferSize,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite
            );

            // Allocate main uniform buffer (device-local for GPU reads)
            m_buffer = Buffer(
                m_device,
                bufferSize,
                vk::BufferUsageFlagBits::eTransferDst |
                vk::BufferUsageFlagBits::eTransferSrc |
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::AllocationCreateFlagBits::eCreateDedicatedMemory
            );
        }

        /**
         * \brief Update the GPU memory of the uniform buffer
         * \param commandBuffer the command buffer used for this operation (must be in recording state)
         *
         * This copies all data from the staging buffer to the device-local buffer.
         */
        void update(vk::CommandBuffer commandBuffer) {
            copyToStageMemory();

            // Copy from staging to device-local buffer
            m_stagingBuffer.copyToBuffer(commandBuffer, m_buffer);
        }

        /**
         * \brief Get the underlying Vulkan buffer handle
         * \return the vk::Buffer handle
         */
        vk::Buffer getBuffer() const {
            return m_buffer.getBuffer();
        }

        /**
         * \brief Implicit conversion to vk::Buffer
         * \return the vk::Buffer handle
         */
        operator vk::Buffer() const {
            return m_buffer.getBuffer();
        }

        /**
         * \brief Get the buffer size in bytes
         * \return the size of the buffer
         */
        size_t size() const {
            return m_variables.data().size();
        }

    private:
        /**
         * \brief Copy data from the dictionary to the staging buffer
         */
        void copyToStageMemory() {
            void* stagingMemory = m_stagingBuffer.map();
            std::memcpy(stagingMemory, m_variables.data().data(), m_variables.data().size());
            m_stagingBuffer.unmap();
        }

        LavaCake::Device m_device;                          ///< Associated device
        Buffer m_buffer;                                     ///< Main uniform buffer (device-local)
        Buffer m_stagingBuffer;                              ///< Staging buffer (host-visible)
        Framework::ByteDictionary m_variables;               ///< Dictionary of variables
    };

} // namespace LavaCake
