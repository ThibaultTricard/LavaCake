#pragma once

#include "Pipeline.hpp"
#include "ShaderModule.hpp"
#include "DescriptorSet.hpp"

namespace LavaCake {
// Compute pipeline for GPU compute workloads
    class ComputePipeline : public Pipeline {
    public:
        // Builder for creating compute pipelines
        class Builder {
        private:

            struct ShaderModuleCreateInfo{
                // to create the shader module
                std::string filepath = "";
                LavaCake::ShadingLanguage lang = ShadingLanguage::eSPIRV;
                bool optimize = false;
                std::vector<std::string> macro;
            };

            LavaCake::Device m_device;
            

            // Shader
            ShaderModuleCreateInfo m_shaderModuleCreateInfo;
            LavaCake::ShaderModule m_shaderModule;
            std::string m_entryPoint = "main";

            bool m_isShaderSet = false;
            
            // Descriptor set layouts and push constants
            std::vector<vk::DescriptorSetLayout> m_descriptorLayouts;
            std::vector<vk::PushConstantRange> m_pushConstants;
            
            // Pipeline cache
            vk::PipelineCache cache = nullptr;
            
            // Specialization constants
            std::vector<vk::SpecializationMapEntry> m_specializationEntries;
            std::vector<uint8_t> m_specializationData;

        public:
            Builder(LavaCake::Device& dev) : m_device(dev){

            }
            
            // Shader configuration
            Builder& setShaderFromFile(const std::string& filepath, const LavaCake::ShadingLanguage language= ShadingLanguage::eGLSL , const std::string& entry = "main") {
                if(!m_isShaderSet){
                    m_shaderModuleCreateInfo.filepath = filepath;
                    m_shaderModuleCreateInfo.lang = language;

                }
                return *this;
            }

            Builder& setOptimization(bool opt) {
                m_shaderModuleCreateInfo.optimize = opt;
                return *this;
            }
            
            Builder& addMacroDefinition(const std::string& macro) {
                m_shaderModuleCreateInfo.macro.push_back(macro);
                return *this;
            }

            // Bindless descriptor sets
            Builder& addDescriptorSetLayout(LavaCake::DescriptorSetLayout& layout) {
                m_descriptorLayouts.push_back(layout.getLayout());
                return *this;
            }

            Builder& setDescriptorSetLayouts(const std::vector<LavaCake::DescriptorSetLayout>& layouts) {
                m_descriptorLayouts = std::vector<vk::DescriptorSetLayout>();
                for(const LavaCake::DescriptorSetLayout& l : layouts) m_descriptorLayouts.push_back(l.getLayout());
                return *this;
            }
            
            Builder& setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts) {
                m_descriptorLayouts = layouts;
                return *this;
            }
            
            // Push constants
            Builder& addPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size) {
                m_pushConstants.push_back(vk::PushConstantRange{stages, offset, size});
                return *this;
            }
            
            template<typename T>
            Builder& addPushConstant(uint32_t offset = 0) {
                return addPushConstantRange(
                    vk::ShaderStageFlagBits::eCompute,
                    offset,
                    sizeof(T)
                );
            }
            
            // Specialization constants
            template<typename T>
            Builder& addSpecializationConstant(uint32_t constantID, const T& value) {
                vk::SpecializationMapEntry entry{};
                entry.constantID = constantID;
                entry.offset = static_cast<uint32_t>(m_specializationData.size());
                entry.size = sizeof(T);
                
                m_specializationEntries.push_back(entry);
                
                // Add data
                const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(&value);
                m_specializationData.insert(
                    m_specializationData.end(),
                    dataPtr,
                    dataPtr + sizeof(T)
                );
                
                return *this;
            }
            
            // Pipeline cache
            Builder& setPipelineCache(vk::PipelineCache pipelineCache) {
                cache = pipelineCache;
                return *this;
            }
            
            // Build the pipeline
            ComputePipeline build(){
                ComputePipeline computePipeline(m_device);

                //create Shader module;
                
                m_shaderModule= LavaCake::ShaderModule(m_device, m_shaderModuleCreateInfo.filepath, m_shaderModuleCreateInfo.lang, vk::ShaderStageFlagBits::eCompute, m_shaderModuleCreateInfo.optimize, m_shaderModuleCreateInfo.macro);
                
                // 2. Create shader stage with specialization constants
                vk::SpecializationInfo specializationInfo{};
                if (!m_specializationEntries.empty()) {
                    specializationInfo.mapEntryCount = static_cast<uint32_t>(m_specializationEntries.size());
                    specializationInfo.pMapEntries = m_specializationEntries.data();
                    specializationInfo.dataSize = m_specializationData.size();
                    specializationInfo.pData = m_specializationData.data();
                }
                
                vk::PipelineShaderStageCreateInfo shaderStage{};
                shaderStage.stage = vk::ShaderStageFlagBits::eCompute;
                shaderStage.module = m_shaderModule.getShaderModule();
                shaderStage.pName = m_entryPoint.c_str();
                if (!m_specializationEntries.empty()) {
                    shaderStage.pSpecializationInfo = &specializationInfo;
                }
                
                // 3. Create pipeline layout
                computePipeline.createPipelineLayout(m_descriptorLayouts, m_pushConstants);
                
                // 4. Create compute pipeline
                vk::ComputePipelineCreateInfo pipelineInfo{};
                pipelineInfo.stage = shaderStage;
                pipelineInfo.layout = computePipeline.getLayout();
                
                auto result = m_device.getDevice().createComputePipeline(cache, pipelineInfo);
                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create compute pipeline!");
                }
                
                computePipeline.m_pipeline = result.value;
                
                
                return computePipeline;
            }
        };
        
        ComputePipeline(LavaCake::Device& dev)
            : Pipeline(dev, vk::PipelineBindPoint::eCompute) {}
        
        // Dispatch commands
        void dispatch(const vk::CommandBuffer& cmd, 
                    uint32_t groupCountX,
                    uint32_t groupCountY = 1,
                    uint32_t groupCountZ = 1) const {
            cmd.dispatch(groupCountX, groupCountY, groupCountZ);
        }
        
        void dispatchIndirect(const vk::CommandBuffer& cmd,
                            vk::Buffer buffer,
                            vk::DeviceSize offset) const {
            cmd.dispatchIndirect(buffer, offset);
        }
        
        // Helper to calculate dispatch groups
        static uint32_t calculateGroupCount(uint32_t workSize, uint32_t workGroupSize) {
            return (workSize + workGroupSize - 1) / workGroupSize;
        }
        
        // Dispatch with automatic group calculation
        void dispatchAuto(const vk::CommandBuffer& cmd,
                        uint32_t workSizeX, uint32_t workGroupSizeX,
                        uint32_t workSizeY = 1, uint32_t workGroupSizeY = 1,
                        uint32_t workSizeZ = 1, uint32_t workGroupSizeZ = 1) const {
            dispatch(cmd,
                calculateGroupCount(workSizeX, workGroupSizeX),
                calculateGroupCount(workSizeY, workGroupSizeY),
                calculateGroupCount(workSizeZ, workGroupSizeZ)
            );
        }
    };
}