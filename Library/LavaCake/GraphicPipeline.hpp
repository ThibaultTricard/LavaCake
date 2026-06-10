#pragma once

#include "Pipeline.hpp"
#include "ShaderModule.hpp"
#include <array>

namespace LavaCake {
    /**
     * \brief Graphics pipeline with modern bindless support and dynamic rendering
     */
    class GraphicsPipeline : public Pipeline {
    public:
        /**
         * \brief Vertex input configuration structure
         */
        struct VertexInputInfo {
            std::vector<vk::VertexInputBindingDescription> bindings;    ///< Vertex buffer binding descriptions
            std::vector<vk::VertexInputAttributeDescription> attributes; ///< Vertex attribute descriptions
        };

        /**
         * \brief Builder class for creating graphics pipelines
         */
        class Builder {
        private:
            LavaCake::Device m_device;
            vk::RenderPass m_renderPass;
            uint32_t m_subpass = 0;
            
            /// Color attachment formats for dynamic rendering
            std::vector<vk::Format> m_colorAttachmentFormats;
            vk::Format m_depthAttachmentFormat = vk::Format::eUndefined;     ///< Depth attachment format
            vk::Format m_stencilAttachmentFormat = vk::Format::eUndefined;   ///< Stencil attachment format
            bool m_useDynamicRendering = false;                              ///< Track which rendering mode to use

            /**
             * \brief Shader module creation information structure
             */
            struct ShaderModuleCreateInfo{
                std::string filepath = "";                                   ///< Path to shader file
                LavaCake::ShadingLanguage lang = ShadingLanguage::eSPIRV;   ///< Shading language
                bool optimize = false;                                       ///< Enable shader optimization
                std::vector<std::string> macro;                             ///< Macro definitions
                std::string entryPoint = "main";                            ///< Shader entry point function name
                std::vector<vk::SpecializationMapEntry> specializationEntries; ///< Specialization constant entries
                std::vector<uint8_t> specializationData;                    ///< Specialization constant data
                const uint32_t* spirvCode = nullptr;                        ///< Pointer to embedded SPIR-V bytecode
                size_t spirvSizeInBytes = 0;                                ///< Size of SPIR-V bytecode in bytes
                bool fromBytecode = false;                                  ///< True if shader comes from bytecode, false if from file
                std::string sourceCode = "";                                ///< Inline shader source code
                bool fromSource = false;                                    ///< True if shader comes from inline source
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

            /**
             * \brief Constructor for dynamic rendering mode (modern - no render pass needed)
             * \param dev the device on which the graphics pipeline will be created
             */
            Builder(const LavaCake::Device& dev)
                : m_device(dev), m_useDynamicRendering(true) {
                m_colorBlendAttachments.push_back(getDefaultColorBlendAttachment());
            }

            /**
             * \brief Constructor for legacy render pass mode
             * \param dev the device on which the graphics pipeline will be created
             * \param rp the render pass
             * \param sp the subpass index (default: 0)
             */
            Builder(const LavaCake::Device& dev, const vk::RenderPass& rp, uint32_t sp = 0)
                : m_device(dev), m_renderPass(rp), m_subpass(sp) {
                m_colorBlendAttachments.push_back(getDefaultColorBlendAttachment());
            }

            /**
             * \brief Add a shader stage from a file
             * \param path the path to the shader file
             * \param type the shader stage type
             * \param language the shading language (default: GLSL)
             * \param entry the entry point function name (default: "main")
             * \return reference to this builder for method chaining
             */
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
                last->filepath = path;
                last->entryPoint = entry;
                last->lang = language;
                last->fromBytecode = false;

                return *this;
            }

            /**
             * \brief Add a shader stage from embedded SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode (as uint32_t array)
             * \param sizeInBytes size of the SPIR-V bytecode in bytes
             * \param type the shader stage type
             * \param entry the entry point function name (default: "main")
             * \return reference to this builder for method chaining
             */
            /**
             * \brief Add a shader stage from source code
             * \param source the shader source code
             * \param language the shading language
             * \param type the shader stage type
             * \param entry the entry point function name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& addShaderFromSource(const std::string& source,
                                         LavaCake::ShadingLanguage language,
                                         vk::ShaderStageFlagBits type,
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
                default: return *this;
                }
                last->sourceCode = source;
                last->lang = language;
                last->entryPoint = entry;
                last->fromSource = true;
                last->fromBytecode = false;
                return *this;
            }

            Builder& addShaderFromSpirvByteCode(const uint32_t* spirvCode,
                                                size_t sizeInBytes,
                                                vk::ShaderStageFlagBits type,
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
                last->spirvCode = spirvCode;
                last->spirvSizeInBytes = sizeInBytes;
                last->entryPoint = entry;
                last->fromBytecode = true;

                return *this;
            }

            /**
             * \brief Add a color attachment format for dynamic rendering
             * \param format the color attachment format
             * \return reference to this builder for method chaining
             */
            Builder& addColorAttachmentFormat(vk::Format format) {
                m_colorAttachmentFormats.push_back(format);
                
                // Ensure we have enough blend attachments
                if (m_colorBlendAttachments.size() < m_colorAttachmentFormats.size()) {
                    m_colorBlendAttachments.push_back(getDefaultColorBlendAttachment());
                }
                
                return *this;
            }

            /**
             * \brief Set all color attachment formats for dynamic rendering
             * \param formats the vector of color attachment formats
             * \return reference to this builder for method chaining
             */
            Builder& setColorAttachmentFormats(const std::vector<vk::Format>& formats) {
                m_colorAttachmentFormats = formats;
                
                // Adjust color blend attachments to match
                while (m_colorBlendAttachments.size() < formats.size()) {
                    m_colorBlendAttachments.push_back(getDefaultColorBlendAttachment());
                }
                
                return *this;
            }

            /**
             * \brief Set the depth attachment format for dynamic rendering
             * \param format the depth attachment format
             * \return reference to this builder for method chaining
             */
            Builder& setDepthAttachmentFormat(vk::Format format) {
                m_depthAttachmentFormat = format;
                return *this;
            }

            /**
             * \brief Set the stencil attachment format for dynamic rendering
             * \param format the stencil attachment format
             * \return reference to this builder for method chaining
             */
            Builder& setStencilAttachmentFormat(vk::Format format) {
                m_stencilAttachmentFormat = format;
                return *this;
            }

            /**
             * \brief Set both depth and stencil attachment formats to the same format
             * \param format the depth and stencil attachment format
             * \return reference to this builder for method chaining
             */
            Builder& setDepthStencilFormat(vk::Format format) {
                m_depthAttachmentFormat = format;
                m_stencilAttachmentFormat = format;
                return *this;
            }

            /**
             * \brief Set the vertex input configuration
             * \param info the vertex input information
             * \return reference to this builder for method chaining
             */
            Builder& setVertexInput(const VertexInputInfo& info) {
                m_vertexInput = info;
                return *this;
            }

            /**
             * \brief Configure for bindless rendering with no vertex input
             * \return reference to this builder for method chaining
             */
            Builder& setBindlessVertexInput() {
                m_vertexInput = VertexInputInfo{};
                return *this;
            }

            /**
             * \brief Set the primitive topology and restart enable
             * \param topo the primitive topology
             * \param restart enable primitive restart (default: false)
             * \return reference to this builder for method chaining
             */
            Builder& setTopology(vk::PrimitiveTopology topo, bool restart = false) {
                m_topology = topo;
                m_primitiveRestartEnable = restart;
                return *this;
            }

            /**
             * \brief Set the polygon rasterization mode
             * \param mode the polygon mode (fill, line, or point)
             * \return reference to this builder for method chaining
             */
            Builder& setPolygonMode(vk::PolygonMode mode) {
                m_polygonMode = mode;
                return *this;
            }

            /**
             * \brief Set the face culling mode
             * \param mode the cull mode flags
             * \param face the front face orientation (default: counter-clockwise)
             * \return reference to this builder for method chaining
             */
            Builder& setCullMode(vk::CullModeFlags mode, vk::FrontFace face = vk::FrontFace::eCounterClockwise) {
                m_cullMode = mode;
                m_frontFace = face;
                return *this;
            }

            /**
             * \brief Set the line width for line rasterization
             * \param width the line width (adds dynamic state if not 1.0)
             * \return reference to this builder for method chaining
             */
            Builder& setLineWidth(float width) {
                m_lineWidth = width;
                if (width != 1.0f) {
                    addDynamicState(vk::DynamicState::eLineWidth);
                }
                return *this;
            }

            /**
             * \brief Configure depth testing
             * \param enable enable depth testing
             * \param write enable depth writes (default: true)
             * \param op the depth comparison operator (default: less)
             * \return reference to this builder for method chaining
             */
            Builder& setDepthTest(bool enable, bool write = true, vk::CompareOp op = vk::CompareOp::eLess) {
                m_depthTestEnable = enable;
                m_depthWriteEnable = write;
                m_depthCompareOp = op;
                return *this;
            }

            /**
             * \brief Configure multisampling anti-aliasing
             * \param count the number of samples per pixel
             * \param sampleShading enable sample shading (default: false)
             * \return reference to this builder for method chaining
             */
            Builder& setMultisampling(vk::SampleCountFlagBits count, bool sampleShading = false) {
                m_samples = count;
                m_sampleShadingEnable = sampleShading;
                return *this;
            }

            /**
             * \brief Enable or disable color blending
             * \param enable enable blending for the first color attachment
             * \return reference to this builder for method chaining
             */
            Builder& setBlendMode(bool enable) {
                m_blendEnable = enable;
                if (!m_colorBlendAttachments.empty()) {
                    m_colorBlendAttachments[0].blendEnable = enable;
                }
                return *this;
            }

            /**
             * \brief Set custom color blend attachment states
             * \param attachments the vector of color blend attachment states
             * \return reference to this builder for method chaining
             */
            Builder& setColorBlendAttachments(const std::vector<vk::PipelineColorBlendAttachmentState>& attachments) {
                m_colorBlendAttachments = attachments;
                return *this;
            }

            /**
             * \brief Add a descriptor set layout to the pipeline
             * \param layout the descriptor set layout to add
             * \return reference to this builder for method chaining
             */
            Builder& addDescriptorSetLayout(vk::DescriptorSetLayout layout) {
                m_descriptorLayouts.push_back(layout);
                return *this;
            }

            /**
             * \brief Set all descriptor set layouts for the pipeline
             * \param layouts the vector of descriptor set layouts
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
             * \param stages the shader stages that will use this push constant
             * \param offset the offset in bytes (default: 0)
             * \return reference to this builder for method chaining
             */
            template<typename T>
            Builder& addPushConstant(vk::ShaderStageFlags stages, uint32_t offset = 0) {
                return addPushConstantRange(stages, offset, sizeof(T));
            }

            /**
             * \brief Add a dynamic state to the pipeline
             * \param state the dynamic state to add
             * \return reference to this builder for method chaining
             */
            Builder& addDynamicState(vk::DynamicState state) {
                if (std::find(m_dynamicStates.begin(), m_dynamicStates.end(), state) == m_dynamicStates.end()) {
                    m_dynamicStates.push_back(state);
                }
                return *this;
            }

            /**
             * \brief Set the pipeline cache for faster pipeline creation
             * \param pipelineCache the Vulkan pipeline cache
             * \return reference to this builder for method chaining
             */
            Builder& setPipelineCache(vk::PipelineCache pipelineCache) {
                m_cache = pipelineCache;
                return *this;
            }

            /**
             * \brief Build and create the graphics pipeline
             * \return the constructed GraphicsPipeline
             */
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

                auto compileShader = [&shaderModules,&shaderStages](LavaCake::Device& d, LavaCake::ShaderModule& m, vk::ShaderStageFlagBits type, ShaderModuleCreateInfo& info){
                    if (info.fromBytecode) {
                        m = LavaCake::ShaderModule(
                            d,
                            info.spirvCode,
                            info.spirvSizeInBytes,
                            type
                        );
                    } else if (info.fromSource) {
                        m = LavaCake::ShaderModule(
                            d,
                            info.sourceCode,
                            info.lang,
                            type,
                            false,
                            info.optimize,
                            info.macro
                        );
                    } else {
                        m = LavaCake::ShaderModule(
                            d,
                            info.filepath,
                            info.lang,
                            type,
                            true,
                            info.optimize,
                            info.macro
                        );
                    }

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
                
                vk::PipelineRenderingCreateInfo renderingCreateInfo{};

                if (m_useDynamicRendering) {
                    // Modern: Use dynamic rendering
                    renderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(m_colorAttachmentFormats.size());
                    renderingCreateInfo.pColorAttachmentFormats = m_colorAttachmentFormats.data();
                    renderingCreateInfo.depthAttachmentFormat = m_depthAttachmentFormat;
                    renderingCreateInfo.stencilAttachmentFormat = m_stencilAttachmentFormat;
                    
                    pipelineInfo.pNext = &renderingCreateInfo;
                    pipelineInfo.renderPass = nullptr;  // No render pass!
                    pipelineInfo.subpass = 0;
                } else {
                    // Legacy: Use render pass
                    pipelineInfo.renderPass = m_renderPass;
                    pipelineInfo.subpass = m_subpass;
                }
                
                auto result = m_device.getDevice().createGraphicsPipeline(m_cache, pipelineInfo);
                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create graphics pipeline!");
                }
                
                graphicsPipeline.m_pipeline = result.value;
                
                
                return graphicsPipeline;
            }

        private:
            /**
             * \brief Get the default color blend attachment state with no blending
             * \return the default color blend attachment state
             */
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

        /**
         * \brief Constructs a GraphicsPipeline
         * \param dev the device on which the graphics pipeline will be created
         */
        GraphicsPipeline(const LavaCake::Device& dev)
            : Pipeline(dev, vk::PipelineBindPoint::eGraphics) {}

        /**
         * \brief Draw primitives
         * \param cmd the command buffer
         * \param vertexCount the number of vertices to draw
         * \param instanceCount the number of instances to draw (default: 1)
         * \param firstVertex the index of the first vertex to draw (default: 0)
         * \param firstInstance the instance ID of the first instance to draw (default: 0)
         */
        void draw(const vk::CommandBuffer& cmd, uint32_t vertexCount,
                uint32_t instanceCount = 1, uint32_t firstVertex = 0,
                uint32_t firstInstance = 0) const {
            cmd.draw(vertexCount, instanceCount, firstVertex, firstInstance);
        }

        /**
         * \brief Draw indexed primitives
         * \param cmd the command buffer
         * \param indexCount the number of indices to draw
         * \param instanceCount the number of instances to draw (default: 1)
         * \param firstIndex the base index within the index buffer (default: 0)
         * \param vertexOffset the value added to the vertex index before indexing into the vertex buffer (default: 0)
         * \param firstInstance the instance ID of the first instance to draw (default: 0)
         */
        void drawIndexed(const vk::CommandBuffer& cmd, uint32_t indexCount,
                        uint32_t instanceCount = 1, uint32_t firstIndex = 0,
                        int32_t vertexOffset = 0, uint32_t firstInstance = 0) const {
            cmd.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }

        /**
         * \brief Draw primitives with indirect parameters from a buffer
         * \param cmd the command buffer
         * \param buffer the buffer containing draw parameters
         * \param offset the byte offset into the buffer
         * \param drawCount the number of draws to execute
         * \param stride the byte stride between successive sets of draw parameters
         */
        void drawIndirect(const vk::CommandBuffer& cmd, vk::Buffer buffer,
                        vk::DeviceSize offset, uint32_t drawCount, uint32_t stride) const {
            cmd.drawIndirect(buffer, offset, drawCount, stride);
        }

        /**
         * \brief Draw indexed primitives with indirect parameters from a buffer
         * \param cmd the command buffer
         * \param buffer the buffer containing draw parameters
         * \param offset the byte offset into the buffer
         * \param drawCount the number of draws to execute
         * \param stride the byte stride between successive sets of draw parameters
         */
        void drawIndexedIndirect(const vk::CommandBuffer& cmd, vk::Buffer buffer,
                                vk::DeviceSize offset, uint32_t drawCount, uint32_t stride) const {
            cmd.drawIndexedIndirect(buffer, offset, drawCount, stride);
        }

        /**
         * \brief Draw mesh task primitives (requires VK_EXT_mesh_shader)
         * \param cmd the command buffer
         * \param groupCountX the number of local workgroups to dispatch in the X dimension
         * \param groupCountY the number of local workgroups to dispatch in the Y dimension (default: 1)
         * \param groupCountZ the number of local workgroups to dispatch in the Z dimension (default: 1)
         */
        void drawMeshTasks(const vk::CommandBuffer& cmd, uint32_t groupCountX,
                           uint32_t groupCountY = 1, uint32_t groupCountZ = 1) const {
            cmd.drawMeshTasksEXT(groupCountX, groupCountY, groupCountZ);
        }

        /**
         * \brief Draw mesh task primitives with indirect parameters from a buffer (requires VK_EXT_mesh_shader)
         * \param cmd the command buffer
         * \param buffer the buffer containing draw parameters (VkDrawMeshTasksIndirectCommandEXT structs)
         * \param offset the byte offset into the buffer
         * \param drawCount the number of draws to execute
         * \param stride the byte stride between successive sets of draw parameters
         */
        void drawMeshTasksIndirect(const vk::CommandBuffer& cmd, vk::Buffer buffer,
                                   vk::DeviceSize offset, uint32_t drawCount, uint32_t stride) const {
            cmd.drawMeshTasksIndirectEXT(buffer, offset, drawCount, stride);
        }
    };
}