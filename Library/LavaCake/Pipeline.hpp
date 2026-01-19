#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
#include <string>
#include <optional>

namespace LavaCake{
class Pipeline {
    protected:
        LavaCake::Device m_device;
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
        vk::PipelineBindPoint m_bindPoint;
        
        // Descriptor set layouts for bindless rendering
        std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
        
        // Push constant ranges
        std::vector<vk::PushConstantRange> m_pushConstantRanges;

    public:
        Pipeline(const LavaCake::Device dev, vk::PipelineBindPoint bp)
            : m_bindPoint(bp) {
                m_device = dev;
            }
        
        virtual ~Pipeline() {
            cleanup();
        }
        
        // Delete copy constructor and assignment
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        
        // Allow move
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
        
        // Getters
        vk::Pipeline get() const { return m_pipeline; }
        vk::PipelineLayout getLayout() const { return m_pipelineLayout; }
        vk::PipelineBindPoint getBindPoint() const { return m_bindPoint; }
        
        // Bind pipeline to command buffer
        void bind(const vk::CommandBuffer& cmd) const {
            cmd.bindPipeline(m_bindPoint, m_pipeline);
        }
        
        // Bind descriptor sets
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
        
        // Push constants helper
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
        
        // Cleanup
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
        // Create pipeline layout (common for all pipeline types)
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