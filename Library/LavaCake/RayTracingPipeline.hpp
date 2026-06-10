#pragma once

#include "Pipeline.hpp"
#include "ShaderModule.hpp"
#include "Device.hpp"

#include <map>

namespace LavaCake {

    /**
     * \brief Ray tracing pipeline for GPU ray tracing workloads
     *
     * This class extends the Pipeline base class to provide ray tracing functionality.
     * It manages shader groups (raygen, miss, hit, callable) and provides the traceRays
     * dispatch method.
     *
     * Example usage:
     * \code
     * auto pipeline = RayTracingPipeline::Builder(device)
     *     .addRaygenShaderFromFile("shaders/raygen.rgen")
     *     .addMissShaderFromFile("shaders/miss.rmiss")
     *     .beginHitGroup()
     *         .setClosestHitShaderFromFile("shaders/closesthit.rchit")
     *     .endHitGroup()
     *     .setMaxRecursionDepth(2)
     *     .addDescriptorSetLayout(layout)
     *     .build();
     *
     * // In render loop:
     * pipeline.bind(cmd);
     * pipeline.traceRays(cmd, sbt.getRaygenRegion(), sbt.getMissRegion(),
     *                    sbt.getHitRegion(), sbt.getCallableRegion(),
     *                    width, height, 1);
     * \endcode
     */
    class RayTracingPipeline : public Pipeline {
    public:
        /**
         * \brief Builder class for creating ray tracing pipelines
         *
         * Provides a fluent API for configuring shader groups, pipeline properties,
         * and descriptor layouts.
         */
        class Builder {
        private:
            /**
             * \brief Information about a shader to be compiled
             */
            struct ShaderInfo {
                std::string filepath;
                ShadingLanguage lang = ShadingLanguage::eGLSL;
                bool optimize = true;
                std::vector<std::string> macros;
                std::string entryPoint = "main";
                const uint32_t* spirvCode = nullptr;
                size_t spirvSizeInBytes = 0;
                bool fromBytecode = false;
                std::string sourceCode = "";
                bool fromSource = false;
                vk::ShaderStageFlagBits stage;
            };

            /**
             * \brief Information about a hit group
             */
            struct HitGroupInfo {
                int32_t closestHitIndex = (int32_t) VK_SHADER_UNUSED_KHR;
                int32_t anyHitIndex = (int32_t) VK_SHADER_UNUSED_KHR;
                int32_t intersectionIndex = (int32_t) VK_SHADER_UNUSED_KHR;
            };

        public:
            /**
             * \brief Constructs a Builder for the specified device
             * \param device the LavaCake device
             */
            Builder(const LavaCake::Device& device)
                : m_device(device)
                , m_vkDevice(device.getDevice())
            {}

            // ---------------------------------------------------------------
            // Raygen Shader
            // ---------------------------------------------------------------

            /**
             * \brief Adds a raygen shader from a file
             * \param path path to the shader file
             * \param lang shading language (default: GLSL)
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& addRaygenShaderFromFile(const std::string& path,
                                            ShadingLanguage lang = ShadingLanguage::eGLSL,
                                            const std::string& entry = "main") {
                int32_t index = addShader(vk::ShaderStageFlagBits::eRaygenKHR, path, lang, entry);
                m_raygenShaderIndices.push_back(index);
                return *this;
            }

            /**
             * \brief Adds a raygen shader from SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode
             * \param sizeInBytes size of the bytecode in bytes
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& addRaygenShaderFromSpirvByteCode(const uint32_t* spirvCode,
                                                     size_t sizeInBytes,
                                                     const std::string& entry = "main") {
                int32_t index = addShaderFromBytecode(vk::ShaderStageFlagBits::eRaygenKHR,
                                                     spirvCode, sizeInBytes, entry);
                m_raygenShaderIndices.push_back(index);
                return *this;
            }

            Builder& addRaygenShaderFromSource(const std::string& source,
                                               ShadingLanguage lang = ShadingLanguage::eGLSL,
                                               const std::string& entry = "main") {
                int32_t index = addShaderFromSource(vk::ShaderStageFlagBits::eRaygenKHR, source, lang, entry);
                m_raygenShaderIndices.push_back(index);
                return *this;
            }

            // ---------------------------------------------------------------
            // Miss Shaders
            // ---------------------------------------------------------------

            /**
             * \brief Adds a miss shader from a file
             * \param path path to the shader file
             * \param lang shading language (default: GLSL)
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& addMissShaderFromFile(const std::string& path,
                                          ShadingLanguage lang = ShadingLanguage::eGLSL,
                                          const std::string& entry = "main") {
                int32_t index = addShader(vk::ShaderStageFlagBits::eMissKHR, path, lang, entry);
                m_missShaderIndices.push_back(index);
                return *this;
            }

            /**
             * \brief Adds a miss shader from SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode
             * \param sizeInBytes size of the bytecode in bytes
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& addMissShaderFromSpirvByteCode(const uint32_t* spirvCode,
                                                   size_t sizeInBytes,
                                                   const std::string& entry = "main") {
                int32_t index = addShaderFromBytecode(vk::ShaderStageFlagBits::eMissKHR,
                                                     spirvCode, sizeInBytes, entry);
                m_missShaderIndices.push_back(index);
                return *this;
            }

            Builder& addMissShaderFromSource(const std::string& source,
                                             ShadingLanguage lang = ShadingLanguage::eGLSL,
                                             const std::string& entry = "main") {
                int32_t index = addShaderFromSource(vk::ShaderStageFlagBits::eMissKHR, source, lang, entry);
                m_missShaderIndices.push_back(index);
                return *this;
            }

            // ---------------------------------------------------------------
            // Hit Groups
            // ---------------------------------------------------------------

            /**
             * \brief Begins a new hit group
             * \return reference to this builder for method chaining
             */
            Builder& beginHitGroup() {
                m_hitGroups.push_back(HitGroupInfo{});
                m_currentHitGroup = &m_hitGroups.back();
                return *this;
            }

            /**
             * \brief Sets the closest hit shader for the current hit group
             * \param path path to the shader file
             * \param lang shading language (default: GLSL)
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setClosestHitShaderFromFile(const std::string& path,
                                                ShadingLanguage lang = ShadingLanguage::eGLSL,
                                                const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShader(vk::ShaderStageFlagBits::eClosestHitKHR, path, lang, entry);
                m_currentHitGroup->closestHitIndex = index;
                return *this;
            }

            /**
             * \brief Sets the closest hit shader from SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode
             * \param sizeInBytes size of the bytecode in bytes
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setClosestHitShaderFromSpirvByteCode(const uint32_t* spirvCode,
                                                         size_t sizeInBytes,
                                                         const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShaderFromBytecode(vk::ShaderStageFlagBits::eClosestHitKHR,
                                                     spirvCode, sizeInBytes, entry);
                m_currentHitGroup->closestHitIndex = index;
                return *this;
            }

            Builder& setClosestHitShaderFromSource(const std::string& source,
                                                   ShadingLanguage lang = ShadingLanguage::eGLSL,
                                                   const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShaderFromSource(vk::ShaderStageFlagBits::eClosestHitKHR, source, lang, entry);
                m_currentHitGroup->closestHitIndex = index;
                return *this;
            }

            /**
             * \brief Sets the any hit shader for the current hit group
             * \param path path to the shader file
             * \param lang shading language (default: GLSL)
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setAnyHitShaderFromFile(const std::string& path,
                                            ShadingLanguage lang = ShadingLanguage::eGLSL,
                                            const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShader(vk::ShaderStageFlagBits::eAnyHitKHR, path, lang, entry);
                m_currentHitGroup->anyHitIndex = index;
                return *this;
            }

            /**
             * \brief Sets the any hit shader from SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode
             * \param sizeInBytes size of the bytecode in bytes
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setAnyHitShaderFromSpirvByteCode(const uint32_t* spirvCode,
                                                     size_t sizeInBytes,
                                                     const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShaderFromBytecode(vk::ShaderStageFlagBits::eAnyHitKHR,
                                                     spirvCode, sizeInBytes, entry);
                m_currentHitGroup->anyHitIndex = index;
                return *this;
            }

            Builder& setAnyHitShaderFromSource(const std::string& source,
                                               ShadingLanguage lang = ShadingLanguage::eGLSL,
                                               const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShaderFromSource(vk::ShaderStageFlagBits::eAnyHitKHR, source, lang, entry);
                m_currentHitGroup->anyHitIndex = index;
                return *this;
            }

            /**
             * \brief Sets the intersection shader for the current hit group (procedural geometry)
             * \param path path to the shader file
             * \param lang shading language (default: GLSL)
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setIntersectionShaderFromFile(const std::string& path,
                                                  ShadingLanguage lang = ShadingLanguage::eGLSL,
                                                  const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShader(vk::ShaderStageFlagBits::eIntersectionKHR, path, lang, entry);
                m_currentHitGroup->intersectionIndex = index;
                return *this;
            }

            /**
             * \brief Sets the intersection shader from SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode
             * \param sizeInBytes size of the bytecode in bytes
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& setIntersectionShaderFromSpirvByteCode(const uint32_t* spirvCode,
                                                           size_t sizeInBytes,
                                                           const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShaderFromBytecode(vk::ShaderStageFlagBits::eIntersectionKHR,
                                                     spirvCode, sizeInBytes, entry);
                m_currentHitGroup->intersectionIndex = index;
                return *this;
            }

            Builder& setIntersectionShaderFromSource(const std::string& source,
                                                     ShadingLanguage lang = ShadingLanguage::eGLSL,
                                                     const std::string& entry = "main") {
                if (!m_currentHitGroup) {
                    throw std::runtime_error("Must call beginHitGroup() before setting hit shaders");
                }
                int32_t index = addShaderFromSource(vk::ShaderStageFlagBits::eIntersectionKHR, source, lang, entry);
                m_currentHitGroup->intersectionIndex = index;
                return *this;
            }

            /**
             * \brief Ends the current hit group
             * \return reference to this builder for method chaining
             */
            Builder& endHitGroup() {
                m_currentHitGroup = nullptr;
                return *this;
            }

            // ---------------------------------------------------------------
            // Callable Shaders
            // ---------------------------------------------------------------

            /**
             * \brief Adds a callable shader from a file
             * \param path path to the shader file
             * \param lang shading language (default: GLSL)
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& addCallableShaderFromFile(const std::string& path,
                                              ShadingLanguage lang = ShadingLanguage::eGLSL,
                                              const std::string& entry = "main") {
                int32_t index = addShader(vk::ShaderStageFlagBits::eCallableKHR, path, lang, entry);
                m_callableShaderIndices.push_back(index);
                return *this;
            }

            /**
             * \brief Adds a callable shader from SPIR-V bytecode
             * \param spirvCode pointer to the SPIR-V bytecode
             * \param sizeInBytes size of the bytecode in bytes
             * \param entry entry point name (default: "main")
             * \return reference to this builder for method chaining
             */
            Builder& addCallableShaderFromSpirvByteCode(const uint32_t* spirvCode,
                                                       size_t sizeInBytes,
                                                       const std::string& entry = "main") {
                int32_t index = addShaderFromBytecode(vk::ShaderStageFlagBits::eCallableKHR,
                                                     spirvCode, sizeInBytes, entry);
                m_callableShaderIndices.push_back(index);
                return *this;
            }

            Builder& addCallableShaderFromSource(const std::string& source,
                                                 ShadingLanguage lang = ShadingLanguage::eGLSL,
                                                 const std::string& entry = "main") {
                int32_t index = addShaderFromSource(vk::ShaderStageFlagBits::eCallableKHR, source, lang, entry);
                m_callableShaderIndices.push_back(index);
                return *this;
            }

            // ---------------------------------------------------------------
            // Pipeline Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Sets the maximum ray recursion depth
             * \param depth the maximum recursion depth
             * \return reference to this builder for method chaining
             */
            Builder& setMaxRecursionDepth(uint32_t depth) {
                m_maxRecursionDepth = depth;
                return *this;
            }

            /**
             * \brief Sets the maximum ray payload size
             * \param size the maximum payload size in bytes
             * \return reference to this builder for method chaining
             */
            Builder& setMaxPayloadSize(uint32_t size) {
                m_maxPayloadSize = size;
                return *this;
            }

            /**
             * \brief Sets the maximum ray attribute size
             * \param size the maximum attribute size in bytes
             * \return reference to this builder for method chaining
             */
            Builder& setMaxAttributeSize(uint32_t size) {
                m_maxAttributeSize = size;
                return *this;
            }

            // ---------------------------------------------------------------
            // Descriptor and Push Constant Configuration
            // ---------------------------------------------------------------

            /**
             * \brief Adds a descriptor set layout
             * \param layout the descriptor set layout
             * \return reference to this builder for method chaining
             */
            Builder& addDescriptorSetLayout(vk::DescriptorSetLayout layout) {
                m_descriptorLayouts.push_back(layout);
                return *this;
            }

            /**
             * \brief Sets all descriptor set layouts
             * \param layouts the vector of descriptor set layouts
             * \return reference to this builder for method chaining
             */
            Builder& setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts) {
                m_descriptorLayouts = layouts;
                return *this;
            }

            /**
             * \brief Adds a push constant range
             * \param stages the shader stages that will access this range
             * \param offset the offset in bytes
             * \param size the size in bytes
             * \return reference to this builder for method chaining
             */
            Builder& addPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size) {
                vk::PushConstantRange range{};
                range.stageFlags = stages;
                range.offset = offset;
                range.size = size;
                m_pushConstants.push_back(range);
                return *this;
            }

            /**
             * \brief Adds a typed push constant
             * \tparam T the type of the push constant data
             * \param stages the shader stages that will access this data
             * \param offset the offset in bytes (default: 0)
             * \return reference to this builder for method chaining
             */
            template<typename T>
            Builder& addPushConstant(vk::ShaderStageFlags stages, uint32_t offset = 0) {
                return addPushConstantRange(stages, offset, sizeof(T));
            }

            /**
             * \brief Sets the pipeline cache
             * \param cache the pipeline cache
             * \return reference to this builder for method chaining
             */
            Builder& setPipelineCache(vk::PipelineCache cache) {
                m_cache = cache;
                return *this;
            }

            /**
             * \brief Enables or disables shader optimization
             * \param optimize whether to optimize shaders (default: true)
             * \return reference to this builder for method chaining
             */
            Builder& setOptimization(bool optimize = true) {
                m_optimize = optimize;
                return *this;
            }

            /**
             * \brief Adds a macro definition for shader compilation
             * \param macro the macro definition (e.g., "DEBUG" or "VALUE=1")
             * \return reference to this builder for method chaining
             */
            Builder& addMacroDefinition(const std::string& macro) {
                m_macros.push_back(macro);
                return *this;
            }

            // ---------------------------------------------------------------
            // Build
            // ---------------------------------------------------------------

            /**
             * \brief Builds the ray tracing pipeline
             * \return the constructed RayTracingPipeline
             * \throws std::runtime_error if building fails
             */
            RayTracingPipeline build() {
                RayTracingPipeline pipeline(m_vkDevice);

                // Validate
                if (m_raygenShaderIndices.empty()) {
                    throw std::runtime_error("Ray tracing pipeline requires at least one raygen shader");
                }

                // Compile all shaders
                std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
                std::vector<std::unique_ptr<ShaderModule>> shaderModules;
                std::vector<std::string> entryPointStrings;

                // Reserve to prevent reallocation which would invalidate c_str() pointers
                shaderStages.reserve(m_shaderInfos.size());
                shaderModules.reserve(m_shaderInfos.size());
                entryPointStrings.reserve(m_shaderInfos.size());

                for (auto& info : m_shaderInfos) {
                    std::unique_ptr<ShaderModule> module;
                    if (info.fromBytecode) {
                        module = std::make_unique<ShaderModule>(
                            m_vkDevice, info.spirvCode, info.spirvSizeInBytes, info.stage);
                    } else if (info.fromSource) {
                        module = std::make_unique<ShaderModule>(
                            m_vkDevice, info.sourceCode, info.lang, info.stage,
                            false, info.optimize, info.macros);
                    } else {
                        module = std::make_unique<ShaderModule>(
                            m_vkDevice, info.filepath, info.lang, info.stage,
                            true, info.optimize, info.macros);
                    }

                    entryPointStrings.push_back(info.entryPoint);

                    vk::PipelineShaderStageCreateInfo stageInfo{};
                    stageInfo.stage = info.stage;
                    stageInfo.module = module->getShaderModule();
                    stageInfo.pName = entryPointStrings.back().c_str();
                    shaderStages.push_back(stageInfo);

                    shaderModules.push_back(std::move(module));
                }

                // Create shader groups
                std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;

                // Raygen groups (general type)
                for (int32_t idx : m_raygenShaderIndices) {
                    vk::RayTracingShaderGroupCreateInfoKHR group{};
                    group.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
                    group.generalShader = idx;
                    group.closestHitShader = VK_SHADER_UNUSED_KHR;
                    group.anyHitShader = VK_SHADER_UNUSED_KHR;
                    group.intersectionShader = VK_SHADER_UNUSED_KHR;
                    shaderGroups.push_back(group);
                }

                // Miss groups (general type)
                for (int32_t idx : m_missShaderIndices) {
                    vk::RayTracingShaderGroupCreateInfoKHR group{};
                    group.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
                    group.generalShader = idx;
                    group.closestHitShader = VK_SHADER_UNUSED_KHR;
                    group.anyHitShader = VK_SHADER_UNUSED_KHR;
                    group.intersectionShader = VK_SHADER_UNUSED_KHR;
                    shaderGroups.push_back(group);
                }

                // Hit groups
                for (const auto& hitGroup : m_hitGroups) {
                    vk::RayTracingShaderGroupCreateInfoKHR group{};
                    group.type = (hitGroup.intersectionIndex != VK_SHADER_UNUSED_KHR)
                        ? vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup
                        : vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
                    group.generalShader = VK_SHADER_UNUSED_KHR;
                    group.closestHitShader = hitGroup.closestHitIndex;
                    group.anyHitShader = hitGroup.anyHitIndex;
                    group.intersectionShader = hitGroup.intersectionIndex;
                    shaderGroups.push_back(group);
                }

                // Callable groups (general type)
                for (int32_t idx : m_callableShaderIndices) {
                    vk::RayTracingShaderGroupCreateInfoKHR group{};
                    group.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
                    group.generalShader = idx;
                    group.closestHitShader = VK_SHADER_UNUSED_KHR;
                    group.anyHitShader = VK_SHADER_UNUSED_KHR;
                    group.intersectionShader = VK_SHADER_UNUSED_KHR;
                    shaderGroups.push_back(group);
                }

                // Create pipeline layout
                pipeline.createPipelineLayout(m_descriptorLayouts, m_pushConstants);

                // Create ray tracing pipeline
                vk::RayTracingPipelineCreateInfoKHR pipelineInfo{};
                pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
                pipelineInfo.pStages = shaderStages.data();
                pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
                pipelineInfo.pGroups = shaderGroups.data();
                pipelineInfo.maxPipelineRayRecursionDepth = m_maxRecursionDepth;
                pipelineInfo.layout = pipeline.getLayout();

                auto result = m_vkDevice.createRayTracingPipelineKHR(
                    nullptr, m_cache, pipelineInfo);

                if (result.result != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create ray tracing pipeline!");
                }

                pipeline.m_pipeline = result.value;

                // Store counts for SBT creation
                pipeline.m_raygenCount = static_cast<uint32_t>(m_raygenShaderIndices.size());
                pipeline.m_missCount = static_cast<uint32_t>(m_missShaderIndices.size());
                pipeline.m_hitGroupCount = static_cast<uint32_t>(m_hitGroups.size());
                pipeline.m_callableCount = static_cast<uint32_t>(m_callableShaderIndices.size());

                // Query RT properties for handle sizes
                auto rtProps = m_device.getRayTracingPipelineProperties();
                pipeline.m_handleSize = rtProps.shaderGroupHandleSize;
                pipeline.m_handleAlignment = rtProps.shaderGroupHandleAlignment;

                return pipeline;
            }

        private:
            /**
             * \brief Adds a shader from a file
             */
            int32_t addShader(vk::ShaderStageFlagBits stage, const std::string& path,
                            ShadingLanguage lang, const std::string& entry) {
                ShaderInfo info;
                info.filepath = path;
                info.lang = lang;
                info.stage = stage;
                info.entryPoint = entry;
                info.optimize = m_optimize;
                info.macros = m_macros;
                info.fromBytecode = false;
                m_shaderInfos.push_back(info);
                return static_cast<int32_t>(m_shaderInfos.size() - 1);
            }

            /**
             * \brief Adds a shader from source code
             */
            int32_t addShaderFromSource(vk::ShaderStageFlagBits stage, const std::string& source,
                                        ShadingLanguage lang, const std::string& entry) {
                ShaderInfo info;
                info.sourceCode = source;
                info.lang = lang;
                info.stage = stage;
                info.entryPoint = entry;
                info.optimize = m_optimize;
                info.macros = m_macros;
                info.fromSource = true;
                m_shaderInfos.push_back(info);
                return static_cast<int32_t>(m_shaderInfos.size() - 1);
            }

            /**
             * \brief Adds a shader from SPIR-V bytecode
             */
            int32_t addShaderFromBytecode(vk::ShaderStageFlagBits stage,
                                         const uint32_t* spirvCode, size_t sizeInBytes,
                                         const std::string& entry) {
                ShaderInfo info;
                info.spirvCode = spirvCode;
                info.spirvSizeInBytes = sizeInBytes;
                info.stage = stage;
                info.entryPoint = entry;
                info.fromBytecode = true;
                m_shaderInfos.push_back(info);
                return static_cast<int32_t>(m_shaderInfos.size() - 1);
            }

            const LavaCake::Device& m_device;
            vk::Device m_vkDevice;

            // Shader storage
            std::vector<ShaderInfo> m_shaderInfos;

            // Shader group indices
            std::vector<int32_t> m_raygenShaderIndices;
            std::vector<int32_t> m_missShaderIndices;
            std::vector<HitGroupInfo> m_hitGroups;
            std::vector<int32_t> m_callableShaderIndices;
            HitGroupInfo* m_currentHitGroup = nullptr;

            // Pipeline properties
            uint32_t m_maxRecursionDepth = 1;
            uint32_t m_maxPayloadSize = 0;
            uint32_t m_maxAttributeSize = 0;

            // Descriptor and push constant configuration
            std::vector<vk::DescriptorSetLayout> m_descriptorLayouts;
            std::vector<vk::PushConstantRange> m_pushConstants;

            // Pipeline cache
            vk::PipelineCache m_cache = nullptr;

            // Shader compilation options
            bool m_optimize = true;
            std::vector<std::string> m_macros;
        };

        /**
         * \brief Constructs a RayTracingPipeline
         * \param dev the Vulkan device
         */
        RayTracingPipeline(const vk::Device& dev)
            : Pipeline(dev, vk::PipelineBindPoint::eRayTracingKHR)
        {}

        /**
         * \brief Dispatch ray tracing
         * \param cmd the command buffer
         * \param raygenSBT the raygen shader binding table region
         * \param missSBT the miss shader binding table region
         * \param hitSBT the hit shader binding table region
         * \param callableSBT the callable shader binding table region
         * \param width the width of the ray tracing dispatch
         * \param height the height of the ray tracing dispatch
         * \param depth the depth of the ray tracing dispatch (default: 1)
         */
        void traceRays(const vk::CommandBuffer& cmd,
                      const vk::StridedDeviceAddressRegionKHR& raygenSBT,
                      const vk::StridedDeviceAddressRegionKHR& missSBT,
                      const vk::StridedDeviceAddressRegionKHR& hitSBT,
                      const vk::StridedDeviceAddressRegionKHR& callableSBT,
                      uint32_t width, uint32_t height, uint32_t depth = 1) const {
            cmd.traceRaysKHR(raygenSBT, missSBT, hitSBT, callableSBT, width, height, depth);
        }

        // ---------------------------------------------------------------
        // Shader Group Information (for SBT creation)
        // ---------------------------------------------------------------

        /**
         * \brief Returns the number of raygen shaders
         * \return the raygen shader count
         */
        uint32_t getRaygenShaderCount() const { return m_raygenCount; }

        /**
         * \brief Returns the number of miss shaders
         * \return the miss shader count
         */
        uint32_t getMissShaderCount() const { return m_missCount; }

        /**
         * \brief Returns the number of hit groups
         * \return the hit group count
         */
        uint32_t getHitGroupCount() const { return m_hitGroupCount; }

        /**
         * \brief Returns the number of callable shaders
         * \return the callable shader count
         */
        uint32_t getCallableShaderCount() const { return m_callableCount; }

        /**
         * \brief Returns the total number of shader groups
         * \return the total shader group count
         */
        uint32_t getTotalShaderGroupCount() const {
            return m_raygenCount + m_missCount + m_hitGroupCount + m_callableCount;
        }

        /**
         * \brief Returns the shader group handle size
         * \return the handle size in bytes
         */
        uint32_t getShaderGroupHandleSize() const { return m_handleSize; }

        /**
         * \brief Returns the shader group handle alignment
         * \return the handle alignment in bytes
         */
        uint32_t getShaderGroupHandleAlignment() const { return m_handleAlignment; }

        /**
         * \brief Gets the shader group handles for SBT creation
         * \return vector containing all shader group handles
         */
        std::vector<uint8_t> getShaderGroupHandles() const {
            uint32_t groupCount = getTotalShaderGroupCount();
            uint32_t dataSize = groupCount * m_handleSize;
            std::vector<uint8_t> handles(dataSize);

            auto result = m_device.getRayTracingShaderGroupHandlesKHR(
                m_pipeline, 0, groupCount, dataSize, handles.data());

            if (result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to get ray tracing shader group handles");
            }

            return handles;
        }

    private:
        friend class Builder;

        uint32_t m_raygenCount = 0;
        uint32_t m_missCount = 0;
        uint32_t m_hitGroupCount = 0;
        uint32_t m_callableCount = 0;

        uint32_t m_handleSize = 0;
        uint32_t m_handleAlignment = 0;
    };

} // namespace LavaCake
