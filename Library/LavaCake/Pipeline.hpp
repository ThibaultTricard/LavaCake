#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
#include <string>
#include <optional>

namespace LavaCake{
/**
 * \brief Base class for managing Vulkan pipelines
 */
class Pipeline {
    protected:
        LavaCake::Device m_device;
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
        vk::PipelineBindPoint m_bindPoint;

        /// Descriptor set layouts for bindless rendering
        std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;

        /// Push constant ranges
        std::vector<vk::PushConstantRange> m_pushConstantRanges;

    public:
        /**
         * \brief Constructs a Pipeline
         * \param dev the device on which the pipeline will be created
         * \param bp the pipeline bind point (graphics, compute, or ray tracing)
         */
        Pipeline(const LavaCake::Device dev, vk::PipelineBindPoint bp)
            : m_bindPoint(bp) {
                m_device = dev;
            }

        /**
         * \brief Virtual destructor, cleans up pipeline resources
         */
        virtual ~Pipeline() {
            cleanup();
        }

        /*
        * we delete const copy and const = operator to avoid duplication
        */
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        /**
         * \brief Move constructor
         * \param other the Pipeline to move from
         */
        Pipeline(Pipeline&& other) noexcept
            : m_device(other.m_device)
            , m_pipeline(other.m_pipeline)
            , m_pipelineLayout(other.m_pipelineLayout)
            , m_bindPoint(other.m_bindPoint)
            , m_descriptorSetLayouts(std::move(other.m_descriptorSetLayouts))
            , m_pushConstantRanges(std::move(other.m_pushConstantRanges)) {
            other.m_pipeline = nullptr;
            other.m_pipelineLayout = nullptr;
        }

        /**
         * \brief Move assignment operator
         * \param other the Pipeline to move from
         * \return reference to this Pipeline
         */
        Pipeline& operator=(Pipeline&& other) noexcept {
            if (this != &other) {
                cleanup();
                m_device = other.m_device;
                m_pipeline = other.m_pipeline;
                m_pipelineLayout = other.m_pipelineLayout;
                m_bindPoint = other.m_bindPoint;
                m_descriptorSetLayouts = std::move(other.m_descriptorSetLayouts);
                m_pushConstantRanges = std::move(other.m_pushConstantRanges);
                
                other.m_pipeline = nullptr;
                other.m_pipelineLayout = nullptr;
            }
            return *this;
        }

        /**
         * \brief Returns the underlying Vulkan pipeline
         * \return the vk::Pipeline
         */
        vk::Pipeline get() const { return m_pipeline; }

        /**
         * \brief Returns the pipeline layout
         * \return the vk::PipelineLayout
         */
        vk::PipelineLayout getLayout() const { return m_pipelineLayout; }

        /**
         * \brief Returns the pipeline bind point
         * \return the vk::PipelineBindPoint
         */
        vk::PipelineBindPoint getBindPoint() const { return m_bindPoint; }

        /**
         * \brief Bind the pipeline to a command buffer
         * \param cmd the command buffer to bind to
         */
        void bind(const vk::CommandBuffer& cmd) const {
            cmd.bindPipeline(m_bindPoint, m_pipeline);
        }

        /**
         * \brief Bind descriptor sets to the pipeline
         * \param cmd the command buffer
         * \param sets the vector of descriptor sets to bind
         * \param firstSet the first descriptor set index (default: 0)
         * \param dynamicOffsets the dynamic offsets for dynamic buffers (default: empty)
         */
        void bindDescriptorSets(const vk::CommandBuffer& cmd,
                            const std::vector<vk::DescriptorSet>& sets,
                            uint32_t firstSet = 0,
                            const std::vector<uint32_t>& dynamicOffsets = {}) const {
            cmd.bindDescriptorSets(
                m_bindPoint,
                m_pipelineLayout,
                firstSet,
                sets,
                dynamicOffsets
            );
        }

        /**
         * \brief Push constants to the pipeline
         * \tparam T the type of data to push
         * \param cmd the command buffer
         * \param stages the shader stages to update
         * \param offset the offset in the push constant range
         * \param data the data to push
         */
        template<typename T>
        void pushConstants(const vk::CommandBuffer& cmd,
                        vk::ShaderStageFlags stages,
                        uint32_t offset,
                        const T& data) const {
            cmd.pushConstants(
                m_pipelineLayout,
                stages,
                offset,
                sizeof(T),
                &data
            );
        }

        /**
         * \brief Clean up and destroy pipeline resources
         */
        void cleanup() {
            if (m_pipeline) {
                m_device.getDevice().destroyPipeline(m_pipeline);
                m_pipeline = nullptr;
            }
            if (m_pipelineLayout) {
                m_device.getDevice().destroyPipelineLayout(m_pipelineLayout);
                m_pipelineLayout = nullptr;
            }
            for (auto layout : m_descriptorSetLayouts) {
                if (layout) {
                    m_device.getDevice().destroyDescriptorSetLayout(layout);
                }
            }
            m_descriptorSetLayouts.clear();
        }

    protected:
        /**
         * \brief Create the pipeline layout (common for all pipeline types)
         * \param layouts the descriptor set layouts (default: empty)
         * \param pushConstants the push constant ranges (default: empty)
         */
        void createPipelineLayout(const std::vector<vk::DescriptorSetLayout>& layouts = {},
                                const std::vector<vk::PushConstantRange>& pushConstants = {}) {
            m_descriptorSetLayouts = layouts;
            m_pushConstantRanges = pushConstants;
            
            vk::PipelineLayoutCreateInfo layoutInfo{};
            layoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
            layoutInfo.pSetLayouts = layouts.data();
            layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
            layoutInfo.pPushConstantRanges = pushConstants.data();
            
            m_pipelineLayout = m_device.getDevice().createPipelineLayout(layoutInfo);
        }
    };
}