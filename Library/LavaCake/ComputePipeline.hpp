#pragma once

#include "Pipeline.hpp"
#include "ShaderModule.hpp"
#include "DescriptorSet.hpp"

namespace LavaCake {
/**
 * \brief Compute pipeline for GPU compute workloads
 */
    class ComputePipeline : public Pipeline {
    public:
        /**
         * \brief Builder class for creating compute pipelines
         */
        class Builder {
        private:

            struct ShaderModuleCreateInfo{
                // to create the shader module
                std::string filepath = "";
                LavaCake::ShadingLanguage lang = ShadingLanguage::eSPIRV;
                bool optimize = false;
                std::vector<std::string> macro;
                std::string entryPoint = "main";
                // Specialization constants
                std::vector<vk::SpecializationMapEntry> specializationEntries;
                std::vector<uint8_t> specializationData;
                // For SPIR-V bytecode
                const uint32_t* spirvCode = nullptr;
                size_t spirvSizeInBytes = 0;
                bool fromBytecode = false;
            };

            vk::Device m_device;
            

            // Shader
            ShaderModuleCreateInfo m_shaderModuleCreateInfo;
            LavaCake::ShaderModule m_shaderModule;
            
            
            // Descriptor set layouts and push constants
            std::vector<vk::DescriptorSetLayout> m_descriptorLayouts;
            std::vector<vk::PushConstantRange> m_pushConstants;
            
            // Pipeline cache
            vk::PipelineCache cache = nullptr;
            
            

        public:
            /**
             * \brief Constructs a Builder for a ComputePipeline
             * \param dev the device on which the compute pipeline will be created
             */
            Builder(const vk::Device& dev) : m_device(dev){

            }

            /**
             * \brief Set the compute shader from a file
             * \param filepath the path to the shader file
             * \param language the shading language (default: GLSL)
             * \param entry the entry point function name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setShaderFromFile(const std::string& filepath, const LavaCake::ShadingLanguage language= ShadingLanguage::eGLSL , const std::string& entry = "main") {
                m_shaderModuleCreateInfo.filepath = filepath;
                m_shaderModuleCreateInfo.lang = language;
                m_shaderModuleCreateInfo.entryPoint = entry;
                m_shaderModuleCreateInfo.fromBytecode = false;

                return *this;
            }

            /**
             * \brief Set the compute shader from SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode (as uint32_t array)
             * \param sizeInBytes size of the SPIR-V bytecode in bytes
             * \param entry the entry point function name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setShaderFromSpirvByteCode(const uint32_t* spirvCode, size_t sizeInBytes, const std::string& entry = "main") {
                m_shaderModuleCreateInfo.spirvCode = spirvCode;
                m_shaderModuleCreateInfo.spirvSizeInBytes = sizeInBytes;
                m_shaderModuleCreateInfo.entryPoint = entry;
                m_shaderModuleCreateInfo.fromBytecode = true;

                return *this;
            }

            /**
             * \brief Enable or disable shader optimization
             * \param opt true to enable optimization, false to disable
             * \return reference to this builder for method chaining
             */
            Builder& setOptimization(bool opt) {
                m_shaderModuleCreateInfo.optimize = opt;
                return *this;
            }

            /**
             * \brief Add a macro definition for shader compilation
             * \param macro the macro definition (e.g., "DEBUG" or "MAX_SIZE=100")
             * \return reference to this builder for method chaining
             */
            Builder& addMacroDefinition(const std::string& macro) {
                m_shaderModuleCreateInfo.macro.push_back(macro);
                return *this;
            }

            /**
             * \brief Add a descriptor set layout to the pipeline
             * \param layout the descriptor set layout to add
             * \return reference to this builder for method chaining
             */
            Builder& addDescriptorSetLayout(const vk::DescriptorSetLayout& layout) {
                m_descriptorLayouts.push_back(layout);
                return *this;
            }

            /**
             * \brief Set the descriptor set layouts for the pipeline
             * \param layouts the vector of LavaCake descriptor set layouts
             * \return reference to this builder for method chaining
             */
            Builder& setDescriptorSetLayouts(const std::vector<LavaCake::DescriptorSetLayout>& layouts) {
                m_descriptorLayouts = std::vector<vk::DescriptorSetLayout>();
                for(const LavaCake::DescriptorSetLayout& l : layouts) m_descriptorLayouts.push_back(l.getLayout());
                return *this;
            }

            /**
             * \brief Set the descriptor set layouts for the pipeline
             * \param layouts the vector of Vulkan descriptor set layouts
             * \return reference to this builder for method chaining
             */
            Builder& setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts) {
                m_descriptorLayouts = layouts;
                return *this;
            }

            /**
             * \brief Add a push constant range to the pipeline
             * \param stages the shader stages that will use this push constant
             * \param offset the offset in bytes
             * \param size the size in bytes
             * \return reference to this builder for method chaining
             */
            Builder& addPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size) {
                m_pushConstants.push_back(vk::PushConstantRange{stages, offset, size});
                return *this;
            }

            /**
             * \brief Add a typed push constant to the pipeline
             * \tparam T the type of the push constant
             * \param offset the offset in bytes (default: 0)
             * \return reference to this builder for method chaining
             */
            template<typename T>
            Builder& addPushConstant(uint32_t offset = 0) {
                return addPushConstantRange(
                    vk::ShaderStageFlagBits::eCompute,
                    offset,
                    sizeof(T)
                );
            }

            /**
             * \brief Add a specialization constant to the shader
             * \tparam T the type of the constant
             * \param constantID the constant ID in the shader
             * \param value the value of the constant
             * \return reference to this builder for method chaining
             */
            template<typename T>
            Builder& addSpecializationConstant(uint32_t constantID, const T& value) {
                vk::SpecializationMapEntry entry{};
                entry.constantID = constantID;
                entry.offset = static_cast<uint32_t>(m_shaderModuleCreateInfo.specializationData.size());
                entry.size = sizeof(T);
                
                m_shaderModuleCreateInfo.specializationEntries.push_back(entry);
                
                // Add data
                const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(&value);
                m_shaderModuleCreateInfo.specializationData.insert(
                    m_shaderModuleCreateInfo.specializationData.end(),
                    dataPtr,
                    dataPtr + sizeof(T)
                );
                
                return *this;
            }

            /**
             * \brief Set the pipeline cache for faster pipeline creation
             * \param pipelineCache the Vulkan pipeline cache
             * \return reference to this builder for method chaining
             */
            Builder& setPipelineCache(vk::PipelineCache pipelineCache) {
                cache = pipelineCache;
                return *this;
            }

            /**
             * \brief Build and create the compute pipeline
             * \return the constructed ComputePipeline
             */
            ComputePipeline build(){
                ComputePipeline computePipeline(m_device);

                //create Shader module;
                if (m_shaderModuleCreateInfo.fromBytecode) {
                    m_shaderModule = LavaCake::ShaderModule(
                        m_device,
                        m_shaderModuleCreateInfo.spirvCode,
                        m_shaderModuleCreateInfo.spirvSizeInBytes,
                        vk::ShaderStageFlagBits::eCompute
                    );
                } else {
                    m_shaderModule = LavaCake::ShaderModule(
                        m_device,
                        m_shaderModuleCreateInfo.filepath,
                        m_shaderModuleCreateInfo.lang,
                        vk::ShaderStageFlagBits::eCompute,
                        m_shaderModuleCreateInfo.optimize,
                        m_shaderModuleCreateInfo.macro
                    );
                }
                
                // 2. Create shader stage with specialization constants
                vk::SpecializationInfo specializationInfo{};
                if (!m_shaderModuleCreateInfo.specializationEntries.empty()) {
                    specializationInfo.mapEntryCount = static_cast<uint32_t>(m_shaderModuleCreateInfo.specializationEntries.size());
                    specializationInfo.pMapEntries = m_shaderModuleCreateInfo.specializationEntries.data();
                    specializationInfo.dataSize = m_shaderModuleCreateInfo.specializationData.size();
                    specializationInfo.pData = m_shaderModuleCreateInfo.specializationData.data();
                }
                
                vk::PipelineShaderStageCreateInfo shaderStage{};
                shaderStage.stage = vk::ShaderStageFlagBits::eCompute;
                shaderStage.module = m_shaderModule.getShaderModule();
                shaderStage.pName = m_shaderModuleCreateInfo.entryPoint.c_str();
                if (!m_shaderModuleCreateInfo.specializationEntries.empty()) {
                    shaderStage.pSpecializationInfo = &specializationInfo;
                }
                
                // 3. Create pipeline layout
                computePipeline.createPipelineLayout(m_descriptorLayouts, m_pushConstants);
                
                // 4. Create compute pipeline
                vk::ComputePipelineCreateInfo pipelineInfo{};
                pipelineInfo.stage = shaderStage;
                pipelineInfo.layout = computePipeline.getLayout();
                
                auto result = m_device.createComputePipeline(cache, pipelineInfo);
                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create compute pipeline!");
                }
                
                computePipeline.m_pipeline = result.value;
                
                
                return computePipeline;
            }
        };

        /**
         * \brief Constructs a ComputePipeline
         * \param dev the device on which the compute pipeline will be created
         */
        ComputePipeline(const vk::Device& dev)
            : Pipeline(dev, vk::PipelineBindPoint::eCompute) {}

        /**
         * \brief Dispatch compute work groups
         * \param cmd the command buffer
         * \param groupCountX the number of work groups in X dimension
         * \param groupCountY the number of work groups in Y dimension (default: 1)
         * \param groupCountZ the number of work groups in Z dimension (default: 1)
         */
        void dispatch(const vk::CommandBuffer& cmd,
                    uint32_t groupCountX,
                    uint32_t groupCountY = 1,
                    uint32_t groupCountZ = 1) const {
            cmd.dispatch(groupCountX, groupCountY, groupCountZ);
        }

        /**
         * \brief Dispatch compute work groups with indirect parameters from a buffer
         * \param cmd the command buffer
         * \param buffer the buffer containing dispatch parameters
         * \param offset the offset in the buffer
         */
        void dispatchIndirect(const vk::CommandBuffer& cmd,
                            vk::Buffer buffer,
                            vk::DeviceSize offset) const {
            cmd.dispatchIndirect(buffer, offset);
        }

        /**
         * \brief Calculate the number of work groups needed for a given work size
         * \param workSize the total work size
         * \param workGroupSize the size of each work group
         * \return the number of work groups needed
         */
        static uint32_t calculateGroupCount(uint32_t workSize, uint32_t workGroupSize) {
            return (workSize + workGroupSize - 1) / workGroupSize;
        }

        /**
         * \brief Dispatch with automatic work group calculation
         * \param cmd the command buffer
         * \param workSizeX the total work size in X dimension
         * \param workGroupSizeX the work group size in X dimension
         * \param workSizeY the total work size in Y dimension (default: 1)
         * \param workGroupSizeY the work group size in Y dimension (default: 1)
         * \param workSizeZ the total work size in Z dimension (default: 1)
         * \param workGroupSizeZ the work group size in Z dimension (default: 1)
         */
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