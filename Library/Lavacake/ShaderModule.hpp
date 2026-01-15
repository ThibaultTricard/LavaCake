
#pragma once
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <vector>
#include <string>
#include <stdexcept>

#include <filesystem>
#include <fstream>

namespace LavaCake {


	const TBuiltInResource DEFAULT_BUILT_IN_RESOURCE_LIMIT = {

			/* .MaxLights = */ 32,
			/* .MaxClipPlanes = */ 6,
			/* .MaxTextureUnits = */ 32,
			/* .MaxTextureCoords = */ 32,
			/* .MaxVertexAttribs = */ 64,
			/* .MaxVertexUniformComponents = */ 4096,
			/* .MaxVaryingFloats = */ 64,
			/* .MaxVertexTextureImageUnits = */ 32,
			/* .MaxCombinedTextureImageUnits = */ 80,
			/* .MaxTextureImageUnits = */ 32,
			/* .MaxFragmentUniformComponents = */ 4096,
			/* .MaxDrawBuffers = */ 32,
			/* .MaxVertexUniformVectors = */ 128,
			/* .MaxVaryingVectors = */ 8,
			/* .MaxFragmentUniformVectors = */ 16,
			/* .MaxVertexOutputVectors = */ 16,
			/* .MaxFragmentInputVectors = */ 15,
			/* .MinProgramTexelOffset = */ -8,
			/* .MaxProgramTexelOffset = */ 7,
			/* .MaxClipDistances = */ 8,
			/* .MaxComputeWorkGroupCountX = */ 65535,
			/* .MaxComputeWorkGroupCountY = */ 65535,
			/* .MaxComputeWorkGroupCountZ = */ 65535,
			/* .MaxComputeWorkGroupSizeX = */ 1024,
			/* .MaxComputeWorkGroupSizeY = */ 1024,
			/* .MaxComputeWorkGroupSizeZ = */ 64,
			/* .MaxComputeUniformComponents = */ 1024,
			/* .MaxComputeTextureImageUnits = */ 16,
			/* .MaxComputeImageUniforms = */ 8,
			/* .MaxComputeAtomicCounters = */ 8,
			/* .MaxComputeAtomicCounterBuffers = */ 1,
			/* .MaxVaryingComponents = */ 60,
			/* .MaxVertexOutputComponents = */ 64,
			/* .MaxGeometryInputComponents = */ 64,
			/* .MaxGeometryOutputComponents = */ 128,
			/* .MaxFragmentInputComponents = */ 128,
			/* .MaxImageUnits = */ 8,
			/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
			/* .MaxCombinedShaderOutputResources = */ 8,
			/* .MaxImageSamples = */ 0,
			/* .MaxVertexImageUniforms = */ 0,
			/* .MaxTessControlImageUniforms = */ 0,
			/* .MaxTessEvaluationImageUniforms = */ 0,
			/* .MaxGeometryImageUniforms = */ 0,
			/* .MaxFragmentImageUniforms = */ 8,
			/* .MaxCombinedImageUniforms = */ 8,
			/* .MaxGeometryTextureImageUnits = */ 16,
			/* .MaxGeometryOutputVertices = */ 256,
			/* .MaxGeometryTotalOutputComponents = */ 1024,
			/* .MaxGeometryUniformComponents = */ 1024,
			/* .MaxGeometryVaryingComponents = */ 64,
			/* .MaxTessControlInputComponents = */ 128,
			/* .MaxTessControlOutputComponents = */ 128,
			/* .MaxTessControlTextureImageUnits = */ 16,
			/* .MaxTessControlUniformComponents = */ 1024,
			/* .MaxTessControlTotalOutputComponents = */ 4096,
			/* .MaxTessEvaluationInputComponents = */ 128,
			/* .MaxTessEvaluationOutputComponents = */ 128,
			/* .MaxTessEvaluationTextureImageUnits = */ 16,
			/* .MaxTessEvaluationUniformComponents = */ 1024,
			/* .MaxTessPatchComponents = */ 120,
			/* .MaxPatchVertices = */ 32,
			/* .MaxTessGenLevel = */ 64,
			/* .MaxViewports = */ 16,
			/* .MaxVertexAtomicCounters = */ 0,
			/* .MaxTessControlAtomicCounters = */ 0,
			/* .MaxTessEvaluationAtomicCounters = */ 0,
			/* .MaxGeometryAtomicCounters = */ 0,
			/* .MaxFragmentAtomicCounters = */ 8,
			/* .MaxCombinedAtomicCounters = */ 8,
			/* .MaxAtomicCounterBindings = */ 1,
			/* .MaxVertexAtomicCounterBuffers = */ 0,
			/* .MaxTessControlAtomicCounterBuffers = */ 0,
			/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
			/* .MaxGeometryAtomicCounterBuffers = */ 0,
			/* .MaxFragmentAtomicCounterBuffers = */ 1,
			/* .MaxCombinedAtomicCounterBuffers = */ 1,
			/* .MaxAtomicCounterBufferSize = */ 16384,
			/* .MaxTransformFeedbackBuffers = */ 4,
			/* .MaxTransformFeedbackInterleavedComponents = */ 64,
			/* .MaxCullDistances = */ 8,
			/* .MaxCombinedClipAndCullDistances = */ 8,
			/* .MaxSamples = */ 4,
			/* .maxMeshOutputVerticesNV = */ 256,
			/* .maxMeshOutputPrimitivesNV = */ 512,
			/* .maxMeshWorkGroupSizeX_NV = */ 32,
			/* .maxMeshWorkGroupSizeY_NV = */ 1,
			/* .maxMeshWorkGroupSizeZ_NV = */ 1,
			/* .maxTaskWorkGroupSizeX_NV = */ 32,
			/* .maxTaskWorkGroupSizeY_NV = */ 1,
			/* .maxTaskWorkGroupSizeZ_NV = */ 1,
			/* .maxMeshViewCountNV = */ 4,
			/* maxMeshOutputVerticesEXT;*/256,
			/* maxMeshOutputPrimitivesEXT;*/512,
			/* maxMeshWorkGroupSizeX_EXT;*/32,
			/* maxMeshWorkGroupSizeY_EXT;*/1,
			/* maxMeshWorkGroupSizeZ_EXT;*/32,
			/* maxTaskWorkGroupSizeX_EXT;*/1,
			/* maxTaskWorkGroupSizeY_EXT;*/1,
			/* maxTaskWorkGroupSizeZ_EXT;*/1,
			/* maxMeshViewCountEXT;*/1,
			/* .maxDualSourceDrawBuffersEXT = */1,
			/* .limits = */
            {
                    /* .nonInductiveForLoops = */ 1,
                    /* .whileLoops = */ 1,
                    /* .doWhileLoops = */ 1,
                    /* .generalUniformIndexing = */ 1,
                    /* .generalAttributeMatrixVectorIndexing = */ 1,
                    /* .generalVaryingIndexing = */ 1,
                    /* .generalSamplerIndexing = */ 1,
                    /* .generalVariableIndexing = */ 1,
                    /* .generalConstantMatrixVectorIndexing = */ 1,
			}};
    
    struct GlslangInitializer {
        GlslangInitializer()  { glslang::InitializeProcess(); }
        ~GlslangInitializer() { glslang::FinalizeProcess(); }
    };

    static GlslangInitializer initializer;

    // Create one global or in main()
    static GlslangInitializer glslangInit;


    EShLanguage toEShLanguage(vk::ShaderStageFlagBits stage)
    {
        switch (stage)
        {
        case vk::ShaderStageFlagBits::eVertex:
            return EShLangVertex;
        case vk::ShaderStageFlagBits::eFragment:
            return EShLangFragment;
        case vk::ShaderStageFlagBits::eCompute:
            return EShLangCompute;
        case vk::ShaderStageFlagBits::eGeometry:
            return EShLangGeometry;
        case vk::ShaderStageFlagBits::eTessellationControl:
            return EShLangTessControl;
        case vk::ShaderStageFlagBits::eTessellationEvaluation:
            return EShLangTessEvaluation;

        // Mesh shading
        case vk::ShaderStageFlagBits::eTaskEXT:
            return EShLangTask;

        case vk::ShaderStageFlagBits::eMeshEXT:
            return EShLangMesh;

        // Ray tracing
        case vk::ShaderStageFlagBits::eRaygenKHR:
            return EShLangRayGen;
        case vk::ShaderStageFlagBits::eAnyHitKHR:
            return EShLangAnyHit;
        case vk::ShaderStageFlagBits::eClosestHitKHR:
            return EShLangClosestHit;
        case vk::ShaderStageFlagBits::eMissKHR:
            return EShLangMiss;
        case vk::ShaderStageFlagBits::eIntersectionKHR:
            return EShLangIntersect;
        case vk::ShaderStageFlagBits::eCallableKHR:
            return EShLangCallable;

        default:
            throw std::runtime_error("Unsupported shader stage");
        }
    }


    std::vector<uint32_t> compileGLSLtoSPIRV(
    const std::string& source,
    vk::ShaderStageFlagBits stage,
    const std::string& filename)
    {
        std::vector<uint32_t> spirv;
    
        EShLanguage language = toEShLanguage(stage);

        const char* strings[] = { source.c_str() };

        glslang::TShader shader(language);
        shader.setStrings(strings, 1);
        shader.setEntryPoint("main");
        shader.setSourceEntryPoint("main");

    
        shader.setEnvInput(
            glslang::EShSourceGlsl,
            language,
            glslang::EShClientVulkan,
            130);

        shader.setEnvClient(
            glslang::EShClientVulkan,
            glslang::EShTargetVulkan_1_3);

        shader.setEnvTarget(
            glslang::EShTargetSpv,
            glslang::EShTargetSpv_1_6);

        if (!shader.parse(
            &DEFAULT_BUILT_IN_RESOURCE_LIMIT,
            460,
            false,
            EShMessages(EShMsgVulkanRules | EShMsgSpvRules)))
        {
            throw std::runtime_error(
                std::string("GLSL parse error in ") + filename + ":\n" +
                shader.getInfoLog());
        }
        
        glslang::TProgram program;
        program.addShader(&shader);
       
        if (!program.link(EShMsgDefault))
            throw std::runtime_error(program.getInfoLog());

        
        glslang::SpvOptions options{};
        options.generateDebugInfo = true;
        
        glslang::GlslangToSpv(
            *program.getIntermediate(language),
            spirv,
            &options);
        
        /**/
        
        return spirv;
    }

    vk::ShaderModule createShaderModuleFromGLSL(
        LavaCake::Device& device,
        const std::string& filename,
        vk::ShaderStageFlagBits stage)
        {
            auto size = std::filesystem::file_size(filename);
            std::string content(size, '\0');
            std::ifstream in(filename);
            in.read(&content[0], size);

            auto spirv = compileGLSLtoSPIRV(content, stage, filename);

            return device.getDevice().createShaderModule(
                vk::ShaderModuleCreateInfo{
                    {},
                    spirv.size() * sizeof(uint32_t),
                    spirv.data()
                });
        }
    
}