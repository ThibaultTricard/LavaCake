#pragma once

#include "Pipeline.hpp"
#include "ShaderModule.hpp"
#include <array>

namespace LavaCake {
    // Graphics pipeline builder with modern bindless support
    class GraphicsPipeline : public Pipeline {
    public:
        // Vertex input configuration
        struct VertexInputInfo {
            std::vector<vk::VertexInputBindingDescription> bindings;
            std::vector<vk::VertexInputAttributeDescription> attributes;
        };
        
        // Builder for creating graphics pipelines
        class Builder {
        private:
            LavaCake::Device m_device;
            vk::RenderPass m_renderPass;
            uint32_t m_subpass = 0;
            

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
            };


            // Shaders
            ShaderModuleCreateInfo m_vertexShaderCreateInfo = {};
            ShaderModule m_vertexModule;
            bool m_isSetVertex = false;

            ShaderModuleCreateInfo m_geometryShaderCreateInfo = {};
            ShaderModule m_geometryModule;
            bool m_isSetGeometry = false;

            ShaderModuleCreateInfo m_tessellationEvaluationShaderCreateInfo = {};
            ShaderModule m_tessellationEvaluationModule;
            bool m_isSetTessellationEvaluation = false;

            ShaderModuleCreateInfo m_tessellationControlShaderCreateInfo = {};
            ShaderModule m_tessellationControlModule;
            bool m_isSetTessellationControl = false;

            ShaderModuleCreateInfo m_fragmentShaderCreateInfo = {};
            ShaderModule m_fragmentModule;
            bool m_isSetFragment = false;

            ShaderModuleCreateInfo m_taskShaderCreateInfo = {};
            ShaderModule m_taskModule;
            bool m_isSetTask = false;

            ShaderModuleCreateInfo m_meshShaderCreateInfo = {};
            ShaderModule m_meshModule;
            bool m_isSetMesh = false;

            ShaderModuleCreateInfo* last;
            
            // Vertex input (optional for bindless)
            VertexInputInfo m_vertexInput;
            
            // Input assembly
            vk::PrimitiveTopology m_topology = vk::PrimitiveTopology::eTriangleList;
            bool m_primitiveRestartEnable = false;
            
            // Viewport and scissor (dynamic by default)
            std::vector<vk::Viewport> m_viewports;
            std::vector<vk::Rect2D> m_scissors;
            bool m_dynamicViewport = true;
            bool m_dynamicScissor = true;
            
            // Rasterization
            vk::PolygonMode m_polygonMode = vk::PolygonMode::eFill;
            vk::CullModeFlags m_cullMode = vk::CullModeFlagBits::eBack;
            vk::FrontFace m_frontFace = vk::FrontFace::eCounterClockwise;
            float m_lineWidth = 1.0f;
            bool m_depthClampEnable = false;
            bool m_rasterizerDiscardEnable = false;
            bool m_depthBiasEnable = false;
            
            // Multisampling
            vk::SampleCountFlagBits m_samples = vk::SampleCountFlagBits::e1;
            bool m_sampleShadingEnable = false;
            float m_minSampleShading = 1.0f;
            
            // Depth and stencil
            bool m_depthTestEnable = true;
            bool m_depthWriteEnable = true;
            vk::CompareOp m_depthCompareOp = vk::CompareOp::eLess;
            bool m_stencilTestEnable = false;
            
            // Color blending
            std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachments;
            bool m_blendEnable = false;
            
            // Dynamic states
            std::vector<vk::DynamicState> m_dynamicStates = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };
            
            // Descriptor set layouts and push constants
            std::vector<vk::DescriptorSetLayout> m_descriptorLayouts;
            std::vector<vk::PushConstantRange> m_pushConstants;
            
            // Pipeline cache
            vk::PipelineCache m_cache = nullptr;

        public:
            Builder(const LavaCake::Device& dev, const vk::RenderPass& rp, uint32_t sp = 0)
                : m_device(dev), m_renderPass(rp), m_subpass(sp) {
                // Default color blend attachment (no blending)
                m_colorBlendAttachments.push_back(getDefaultColorBlendAttachment());
            }
            
            // Shader configuration
            Builder& addShaderFromFile(const std::string& path, 
                                    vk::ShaderStageFlagBits type,
                                    const LavaCake::ShadingLanguage language= ShadingLanguage::eGLSL,
                                    const std::string& entry = "main") {

                switch (type)
                {
                case vk::ShaderStageFlagBits::eVertex: last = &m_vertexShaderCreateInfo; m_isSetVertex = true; break;
                case vk::ShaderStageFlagBits::eTessellationControl: last = &m_tessellationControlShaderCreateInfo; m_isSetTessellationControl = true; break;
                case vk::ShaderStageFlagBits::eTessellationEvaluation: last = &m_tessellationEvaluationShaderCreateInfo; m_isSetTessellationEvaluation = true; break;
                case vk::ShaderStageFlagBits::eGeometry: last = &m_geometryShaderCreateInfo; m_isSetGeometry = true; break;
                case vk::ShaderStageFlagBits::eFragment: last = &m_fragmentShaderCreateInfo; m_isSetFragment = true; break;
                case vk::ShaderStageFlagBits::eTaskEXT: last = &m_taskShaderCreateInfo; m_isSetTask = true; break;
                case vk::ShaderStageFlagBits::eMeshEXT: last = &m_meshShaderCreateInfo; m_isSetMesh = true; break;
                
                default:
                    return *this;
                    break;
                }
                last->filepath =path;
                last->entryPoint = entry;
            
                return *this;
            }
            
            // Vertex input (optional for bindless - can use shader inputs instead)
            Builder& setVertexInput(const VertexInputInfo& info) {
                m_vertexInput = info;
                return *this;
            }
            
            // For bindless: no vertex input needed
            Builder& setBindlessVertexInput() {
                m_vertexInput = VertexInputInfo{};
                return *this;
            }
            
            // Input assembly
            Builder& setTopology(vk::PrimitiveTopology topo, bool restart = false) {
                m_topology = topo;
                m_primitiveRestartEnable = restart;
                return *this;
            }
            
            // Rasterization
            Builder& setPolygonMode(vk::PolygonMode mode) {
                m_polygonMode = mode;
                return *this;
            }
            
            Builder& setCullMode(vk::CullModeFlags mode, vk::FrontFace face = vk::FrontFace::eCounterClockwise) {
                m_cullMode = mode;
                m_frontFace = face;
                return *this;
            }
            
            Builder& setLineWidth(float width) {
                m_lineWidth = width;
                if (width != 1.0f) {
                    addDynamicState(vk::DynamicState::eLineWidth);
                }
                return *this;
            }
            
            // Depth testing
            Builder& setDepthTest(bool enable, bool write = true, vk::CompareOp op = vk::CompareOp::eLess) {
                m_depthTestEnable = enable;
                m_depthWriteEnable = write;
                m_depthCompareOp = op;
                return *this;
            }
            
            // Multisampling
            Builder& setMultisampling(vk::SampleCountFlagBits count, bool sampleShading = false) {
                m_samples = count;
                m_sampleShadingEnable = sampleShading;
                return *this;
            }
            
            // Color blending
            Builder& setBlendMode(bool enable) {
                m_blendEnable = enable;
                if (!m_colorBlendAttachments.empty()) {
                    m_colorBlendAttachments[0].blendEnable = enable;
                }
                return *this;
            }
            
            Builder& setColorBlendAttachments(const std::vector<vk::PipelineColorBlendAttachmentState>& attachments) {
                m_colorBlendAttachments = attachments;
                return *this;
            }
            
            // Bindless descriptor sets
            Builder& addDescriptorSetLayout(vk::DescriptorSetLayout layout) {
                m_descriptorLayouts.push_back(layout);
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
            Builder& addPushConstant(vk::ShaderStageFlags stages, uint32_t offset = 0) {
                return addPushConstantRange(stages, offset, sizeof(T));
            }
            
            // Dynamic states
            Builder& addDynamicState(vk::DynamicState state) {
                if (std::find(m_dynamicStates.begin(), m_dynamicStates.end(), state) == m_dynamicStates.end()) {
                    m_dynamicStates.push_back(state);
                }
                return *this;
            }
            
            // Pipeline cache
            Builder& setPipelineCache(vk::PipelineCache pipelineCache) {
                m_cache = pipelineCache;
                return *this;
            }
            
            // Build the pipeline
            GraphicsPipeline build(){
                auto graphicsPipeline = GraphicsPipeline(m_device);
                
                // Check if their is incompatibility in the shader list
                if(m_isSetVertex || m_isSetMesh || m_isSetTask){
                    if(m_isSetVertex && (m_isSetMesh || m_isSetTask)){
                        throw std::runtime_error("A pipeline that contains a mesh or a task shader cannot contain a vertex shader.");
                    }

                }
                else{
                    throw std::runtime_error("No vertex, task or mesh shader present, the pipeline cannot be created.");
                }


                // 1. Compile shaders
                std::vector<vk::ShaderModule> shaderModules;
                std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

                auto compileShader = [&shaderModules,&shaderStages](LavaCake::Device d, LavaCake::ShaderModule& m, vk::ShaderStageFlagBits type, ShaderModuleCreateInfo info){
                    m = LavaCake::ShaderModule(
                        d,
                        info.filepath,
                        info.lang,
                        type,
                        info.optimize,
                        info.macro
                    );
                    vk::SpecializationInfo specializationInfo{};
                    if (!info.specializationEntries.empty()) {
                        specializationInfo.mapEntryCount = static_cast<uint32_t>(info.specializationEntries.size());
                        specializationInfo.pMapEntries = info.specializationEntries.data();
                        specializationInfo.dataSize = info.specializationData.size();
                        specializationInfo.pData = info.specializationData.data();
                    }

                    vk::PipelineShaderStageCreateInfo stageInfo{};
                    stageInfo.stage = type;
                    stageInfo.module = m.getShaderModule();
                    stageInfo.pName = info.entryPoint.c_str();
                    if (!info.specializationEntries.empty()) {
                        stageInfo.pSpecializationInfo = &specializationInfo;
                    }

                    shaderModules.push_back(m.getShaderModule());
                    shaderStages.push_back(stageInfo);
                };

                if(m_isSetVertex){
                    compileShader(m_device, m_vertexModule, vk::ShaderStageFlagBits::eVertex, m_vertexShaderCreateInfo );
                }
                if(m_isSetTessellationControl){
                    compileShader(m_device, m_tessellationControlModule, vk::ShaderStageFlagBits::eTessellationControl, m_tessellationControlShaderCreateInfo );
                }
                if(m_isSetTessellationEvaluation){
                    compileShader(m_device, m_tessellationEvaluationModule, vk::ShaderStageFlagBits::eTessellationEvaluation, m_tessellationEvaluationShaderCreateInfo );
                }
                if(m_isSetGeometry){
                    compileShader(m_device, m_geometryModule, vk::ShaderStageFlagBits::eGeometry, m_geometryShaderCreateInfo );
                }
                if(m_isSetFragment){
                    compileShader(m_device, m_fragmentModule, vk::ShaderStageFlagBits::eFragment, m_fragmentShaderCreateInfo );
                }

                if(m_isSetTask){
                    compileShader(m_device, m_taskModule, vk::ShaderStageFlagBits::eTaskEXT, m_taskShaderCreateInfo );
                }
                if(m_isSetMesh){
                    compileShader(m_device, m_meshModule, vk::ShaderStageFlagBits::eMeshEXT, m_meshShaderCreateInfo );
                }
                
                
                
                // 2. Vertex input state
                vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
                vertexInputInfo.vertexBindingDescriptionCount = 
                    static_cast<uint32_t>(m_vertexInput.bindings.size());
                vertexInputInfo.pVertexBindingDescriptions = m_vertexInput.bindings.data();
                vertexInputInfo.vertexAttributeDescriptionCount = 
                    static_cast<uint32_t>(m_vertexInput.attributes.size());
                vertexInputInfo.pVertexAttributeDescriptions = m_vertexInput.attributes.data();
                
                // 3. Input assembly
                vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
                inputAssembly.topology = m_topology;
                inputAssembly.primitiveRestartEnable = m_primitiveRestartEnable;
                
                // 4. Viewport state
                vk::PipelineViewportStateCreateInfo viewportState{};
                if (m_viewports.empty()) {
                    // Dynamic viewport - just set count
                    viewportState.viewportCount = 1;
                    viewportState.scissorCount = 1;
                } else {
                    viewportState.viewportCount = static_cast<uint32_t>(m_viewports.size());
                    viewportState.pViewports = m_viewports.data();
                    viewportState.scissorCount = static_cast<uint32_t>(m_scissors.size());
                    viewportState.pScissors = m_scissors.data();
                }
                
                // 5. Rasterization state
                vk::PipelineRasterizationStateCreateInfo rasterizer{};
                rasterizer.depthClampEnable = m_depthClampEnable;
                rasterizer.rasterizerDiscardEnable = m_rasterizerDiscardEnable;
                rasterizer.polygonMode = m_polygonMode;
                rasterizer.lineWidth = m_lineWidth;
                rasterizer.cullMode = m_cullMode;
                rasterizer.frontFace = m_frontFace;
                rasterizer.depthBiasEnable = m_depthBiasEnable;
                
                // 6. Multisampling
                vk::PipelineMultisampleStateCreateInfo multisampling{};
                multisampling.sampleShadingEnable = m_sampleShadingEnable;
                multisampling.rasterizationSamples = m_samples;
                multisampling.minSampleShading = m_minSampleShading;
                
                // 7. Depth and stencil
                vk::PipelineDepthStencilStateCreateInfo depthStencil{};
                depthStencil.depthTestEnable = m_depthTestEnable;
                depthStencil.depthWriteEnable = m_depthWriteEnable;
                depthStencil.depthCompareOp = m_depthCompareOp;
                depthStencil.depthBoundsTestEnable = false;
                depthStencil.stencilTestEnable = m_stencilTestEnable;
                
                // 8. Color blending
                vk::PipelineColorBlendStateCreateInfo colorBlending{};
                colorBlending.logicOpEnable = false;
                colorBlending.logicOp = vk::LogicOp::eCopy;
                colorBlending.attachmentCount = static_cast<uint32_t>(m_colorBlendAttachments.size());
                colorBlending.pAttachments = m_colorBlendAttachments.data();
                
                // 9. Dynamic state
                vk::PipelineDynamicStateCreateInfo dynamicState{};
                dynamicState.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
                dynamicState.pDynamicStates = m_dynamicStates.data();
                
                // 10. Create pipeline layout
                graphicsPipeline.createPipelineLayout(m_descriptorLayouts, m_pushConstants);
                
                // 11. Create graphics pipeline
                vk::GraphicsPipelineCreateInfo pipelineInfo{};
                pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
                pipelineInfo.pStages = shaderStages.data();
                pipelineInfo.pVertexInputState = &vertexInputInfo;
                pipelineInfo.pInputAssemblyState = &inputAssembly;
                pipelineInfo.pViewportState = &viewportState;
                pipelineInfo.pRasterizationState = &rasterizer;
                pipelineInfo.pMultisampleState = &multisampling;
                pipelineInfo.pDepthStencilState = &depthStencil;
                pipelineInfo.pColorBlendState = &colorBlending;
                pipelineInfo.pDynamicState = &dynamicState;
                pipelineInfo.layout = graphicsPipeline.getLayout();
                pipelineInfo.renderPass = m_renderPass;
                pipelineInfo.subpass = m_subpass;
                
                auto result = m_device.getDevice().createGraphicsPipeline(m_cache, pipelineInfo);
                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create graphics pipeline!");
                }
                
                graphicsPipeline.m_pipeline = result.value;
                
                
                return graphicsPipeline;
            }
            
        private:
            static vk::PipelineColorBlendAttachmentState getDefaultColorBlendAttachment() {
                vk::PipelineColorBlendAttachmentState attachment{};
                attachment.colorWriteMask = 
                    vk::ColorComponentFlagBits::eR |
                    vk::ColorComponentFlagBits::eG |
                    vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA;
                attachment.blendEnable = false;
                return attachment;
            }
        };
        
        GraphicsPipeline(const LavaCake::Device& dev)
            : Pipeline(dev, vk::PipelineBindPoint::eGraphics) {}
        
        // Draw commands
        void draw(const vk::CommandBuffer& cmd, uint32_t vertexCount, 
                uint32_t instanceCount = 1, uint32_t firstVertex = 0, 
                uint32_t firstInstance = 0) const {
            cmd.draw(vertexCount, instanceCount, firstVertex, firstInstance);
        }
        
        void drawIndexed(const vk::CommandBuffer& cmd, uint32_t indexCount,
                        uint32_t instanceCount = 1, uint32_t firstIndex = 0,
                        int32_t vertexOffset = 0, uint32_t firstInstance = 0) const {
            cmd.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
        
        void drawIndirect(const vk::CommandBuffer& cmd, vk::Buffer buffer,
                        vk::DeviceSize offset, uint32_t drawCount, uint32_t stride) const {
            cmd.drawIndirect(buffer, offset, drawCount, stride);
        }
        
        void drawIndexedIndirect(const vk::CommandBuffer& cmd, vk::Buffer buffer,
                                vk::DeviceSize offset, uint32_t drawCount, uint32_t stride) const {
            cmd.drawIndexedIndirect(buffer, offset, drawCount, stride);
        }
    };
}