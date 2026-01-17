
#pragma once
#include "Device.hpp"

#include <shaderc/shaderc.hpp>
#include <vector>
#include <string>
#include <stdexcept>

#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>


namespace LavaCake {
    enum class ShadingLanguage{
        eSPIRV,
        eGLSL,
    };

    class ShaderModule{
    public :
        ShaderModule() = default;
        
        /*
        * we delete const copy and const = operator to avoid gpu pointer duplication 
        */
        ShaderModule(const ShaderModule& module) = delete;
        ShaderModule& operator=(const ShaderModule&) = delete;

        /**
         * \brief copie constructeur, copie a ShaderModule and make sure the pointer toward gpu memory are not duplicated
         */
        ShaderModule(ShaderModule&& m) noexcept:
            m_shaderModule(std::exchange(m.m_shaderModule, {})),
            m_stage(std::exchange(m.m_stage, vk::ShaderStageFlagBits::eAll)),
            m_filePath(std::exchange(m.m_filePath, "")),
            m_lang(std::exchange(m.m_lang, ShadingLanguage::eSPIRV)),
            m_optimize(std::exchange(m.m_optimize, false)),
            m_macroDefinitions(std::exchange(m.m_macroDefinitions, {})),
            m_device(std::exchange(m.m_device, nullptr))
        {}

        /**
         * \brief copie operator, copie a ShaderModule and make sure the pointer toward gpu memory are not duplicated
         */
        ShaderModule& operator=(ShaderModule&& m) noexcept
        {
            if (this != &m)
            {
            m_shaderModule =std::exchange(m.m_shaderModule, {});
            m_stage =std::exchange(m.m_stage, vk::ShaderStageFlagBits::eAll);
            m_filePath=std::exchange(m.m_filePath, "");
            m_lang=std::exchange(m.m_lang, ShadingLanguage::eSPIRV);
            m_optimize=std::exchange(m.m_optimize, false);
            m_macroDefinitions=std::exchange(m.m_macroDefinitions, {});
            m_device=std::exchange(m.m_device, nullptr);
            }
            return *this;
        }

        /**
         * \brief Create a Shader Module from a shader file
         * \param device the device on which the buffer will be created
         * \param filepath the path of the shader
         * \param lang the shading language
         * \param stage the shading stage,
         * \param optimize does the shader need to be optimized
         * \param macroDefinitions 
         */
        ShaderModule(
            const LavaCake::Device& device,
            const std::string& filepath,
            const ShadingLanguage lang,
            const vk::ShaderStageFlagBits stage,
            bool optimize = true,
            const std::vector<std::string>& macroDefinitions = {}){
                
                m_filePath = filepath;
                m_lang = lang;
                m_stage = stage;
                m_optimize= optimize;
                m_macroDefinitions = macroDefinitions;

                m_device = &device;

                std::string source = readFile(filepath);
                std::vector<uint32_t> spvcode;
                switch (lang){
                    case ShadingLanguage::eSPIRV:
                        spvcode = std::vector<uint32_t>(source.length() * sizeof(char));
                        std::memcpy(spvcode.data(), source.data(), source.length() * sizeof(char));
                        m_shaderModule = createShaderModule(spvcode);
                        break;
                    case ShadingLanguage::eGLSL:
                        m_shaderModule = compileShaderFromGLSLFile(source);
                        break;
                        
                }
            }

        /**
         * \brief getter for the shaderModule handle
         * \return vk::ShaderModule
         */
        const vk::ShaderModule getShaderModule() const{
            return m_shaderModule;
        }


        ~ShaderModule(){
            if (m_shaderModule) {
                m_device->getDevice().destroyShaderModule(m_shaderModule);
            }
        }

    private :
    
    vk::ShaderModule                m_shaderModule;
    vk::ShaderStageFlagBits         m_stage;
    std::string                     m_filePath;
    ShadingLanguage                 m_lang;
    bool                            m_optimize;
    std::vector<std::string>        m_macroDefinitions;

    const LavaCake::Device*         m_device;

    shaderc_shader_kind getShaderKind(const vk::ShaderStageFlagBits stage ) {
        switch ( stage )
        {
            case vk::ShaderStageFlagBits::eVertex: return shaderc_vertex_shader;
            case vk::ShaderStageFlagBits::eTessellationControl: return shaderc_tess_control_shader;
            case vk::ShaderStageFlagBits::eTessellationEvaluation: return shaderc_tess_evaluation_shader;
            case vk::ShaderStageFlagBits::eGeometry: return shaderc_geometry_shader;
            case vk::ShaderStageFlagBits::eFragment: return shaderc_fragment_shader;
            case vk::ShaderStageFlagBits::eCompute: return shaderc_compute_shader;
            case vk::ShaderStageFlagBits::eRaygenKHR: return shaderc_raygen_shader;
            case vk::ShaderStageFlagBits::eAnyHitKHR: return shaderc_anyhit_shader;
            case vk::ShaderStageFlagBits::eClosestHitKHR: return shaderc_closesthit_shader;
            case vk::ShaderStageFlagBits::eMissKHR: return shaderc_miss_shader;
            case vk::ShaderStageFlagBits::eIntersectionKHR: return shaderc_intersection_shader;
            case vk::ShaderStageFlagBits::eCallableKHR: return shaderc_callable_shader;
            case vk::ShaderStageFlagBits::eTaskEXT: return shaderc_task_shader;
            case vk::ShaderStageFlagBits::eMeshEXT: return shaderc_mesh_shader;
            default: assert( false && "Unknown shader stage" ); throw std::runtime_error("Unknown shader type");
        }
                
        }
        
        // Apply macro definitions to compile options
        void applyMacroDefinitions(
            shaderc::CompileOptions& options,
            const std::vector<std::string>& macroDefinitions
        ) {
            for (const auto& macro : macroDefinitions) {
                size_t equalPos = macro.find('=');
                if (equalPos != std::string::npos) {
                    // Macro with value: DEFINE=VALUE
                    std::string name = macro.substr(0, equalPos);
                    std::string value = macro.substr(equalPos + 1);
                    options.AddMacroDefinition(name, value);
                } else {
                    // Macro without value: DEFINE
                    options.AddMacroDefinition(macro, "1");
                }
            }
        }


        // Read file contents
        std::string readFile(const std::string& filepath){
            std::ifstream file(filepath);
            
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open shader file: " + filepath);
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
        
        // Create shader module from SPIR-V code
        vk::ShaderModule createShaderModule(
            const std::vector<uint32_t>& spirvCode
        ){
            
            vk::ShaderModuleCreateInfo createInfo{};
            createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
            createInfo.pCode = spirvCode.data();
            
            
            try {
                return m_device->getDevice().createShaderModule(createInfo);
                std::cout<<"loaded"<< std::endl;
            } catch (vk::SystemError& err) {
                throw std::runtime_error(
                    "Failed to create shader module: " + std::string(err.what())
                );
            }
        }

       
        

        // Get SPIR-V binary from GLSL source (without creating module)
        std::vector<uint32_t> compileToSPIRV(
            const std::string& source
        ){
            shaderc::Compiler compiler;
            shaderc::CompileOptions options;
            
            // Set optimization level
            if (m_optimize) {
                options.SetOptimizationLevel(shaderc_optimization_level_performance);
            } else {
                options.SetOptimizationLevel(shaderc_optimization_level_zero);
                options.SetGenerateDebugInfo();
            }
            
            // Set target environment to Vulkan 1.3 to support all extensions
            options.SetTargetEnvironment(shaderc_target_env_vulkan, 
                                        shaderc_env_version_vulkan_1_3);
            options.SetTargetSpirv(shaderc_spirv_version_1_6);
            
            // Apply macro definitions
            applyMacroDefinitions(options, m_macroDefinitions);
            
            // Enable common extensions for RTX and mesh shading
            // These are typically enabled automatically when used in shader code
            // but can be explicitly enabled if needed
            
            // Compile shader
            shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
                source,
                getShaderKind(m_stage),
                m_filePath.c_str(),
                options
            );
            
            // Check compilation status
            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                throw std::runtime_error(
                    "Shader compilation failed for " + m_filePath + ":\n" +
                    result.GetErrorMessage()
                );
            }
            
            // Get warnings if any
            if (result.GetNumWarnings() > 0) {
                std::cerr << "Shader compilation warnings for " << m_filePath << ":\n"
                        << result.GetErrorMessage() << std::endl;
            }
            
            // Return SPIR-V binary
            return std::vector<uint32_t>(result.cbegin(), result.cend());
        }

        
        // Compile GLSL file to SPIR-V and create shader module
        vk::ShaderModule compileShaderFromGLSLFile(const std::string& source){
            auto spirvCode = compileToSPIRV(source);
            
            return createShaderModule(spirvCode);
        }

        // Get file extension for shader type (for auto-detection)
        std::string getShaderExtension(vk::ShaderStageFlagBits stage){
            switch (stage) {
                case vk::ShaderStageFlagBits::eVertex:                  return ".vert";
                case vk::ShaderStageFlagBits::eFragment:                return ".frag";
                case vk::ShaderStageFlagBits::eGeometry:                return ".geom";
                case vk::ShaderStageFlagBits::eTessellationControl:     return ".tesc";
                case vk::ShaderStageFlagBits::eTessellationEvaluation:  return ".tese";
                case vk::ShaderStageFlagBits::eCompute:                 return ".comp";
                case vk::ShaderStageFlagBits::eTaskEXT:                 return ".task";
                case vk::ShaderStageFlagBits::eMeshEXT:                 return ".mesh";
                case vk::ShaderStageFlagBits::eRaygenKHR:               return ".rgen";
                case vk::ShaderStageFlagBits::eAnyHitKHR:               return ".rahit";
                case vk::ShaderStageFlagBits::eClosestHitKHR:           return ".rchit";
                case vk::ShaderStageFlagBits::eMissKHR:                 return ".rmiss";
                case vk::ShaderStageFlagBits::eIntersectionKHR:         return ".rint";
                case vk::ShaderStageFlagBits::eCallableKHR:             return ".rcall";
                default:                          return ".glsl";
            }
        }
        
    
    
    };
}