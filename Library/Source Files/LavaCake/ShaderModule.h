#pragma once

#include "AllHeaders.h"
#include "Device.h"

namespace LavaCake {
	namespace Framework{

		class ShaderModule {
		public :

			ShaderModule(std::string path, VkShaderStageFlagBits stageBits, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) {
				LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logicalDevice = d->getLogicalDevice();
				if (!GetBinaryFileContents(path, m_spirv)) {
					ErrorCheck::setError(6, "Cannot reed the Shader file ");
				}
				
				InitVkDestroyer(logicalDevice, m_module);
				if (!Shader::CreateShaderModule(logicalDevice, m_spirv, *m_module)) {
					ErrorCheck::setError(6, "Cannot create the Shader module");
				}
				
				m_stageParameter = {
						stageBits,
						* m_module,
						entrypoint,
						specialization
				};
			}

			Shader::ShaderStageParameters&	getStageParameter() {
				return m_stageParameter;
			}

		private :
			VkDestroyer(VkShaderModule)								m_module;
			std::string																m_path;
			std::vector<unsigned char>								m_spirv;
			Shader::ShaderStageParameters							m_stageParameter;
		};

		class VertexShaderModule : public ShaderModule {
		public:
			VertexShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr):
				ShaderModule(path, VK_SHADER_STAGE_VERTEX_BIT, entrypoint, specialization) {

			}
		};

		class TessellationControlShaderModule : public ShaderModule {
		public:
			TessellationControlShaderModule( std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule( path, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, entrypoint, specialization) {

			}
		};

		class TessellationEvaluationShaderModule : public ShaderModule {
		public:
			TessellationEvaluationShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, entrypoint, specialization) {

			}
		};

		class GeometryShaderModule : public ShaderModule {
		public:
			GeometryShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_GEOMETRY_BIT, entrypoint, specialization) {

			}
		};

		class FragmentShaderModule : public ShaderModule {
		public:
			FragmentShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_FRAGMENT_BIT, entrypoint, specialization) {

			}
		};

		class ComputeShaderModule : public ShaderModule {
		public:
			ComputeShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_COMPUTE_BIT, entrypoint, specialization) {

			}
		};
	}
}