#pragma once

#include "AllHeaders.h"
#include "Device.h"

namespace LavaCake {
	namespace Framework{

		struct ShaderStageParameters {
			VkShaderStageFlagBits        ShaderStage;
			VkShaderModule               ShaderModule;
			char const* EntryPointName;
			VkSpecializationInfo const* SpecializationInfo;
		};

    
		class ShaderModule {
		protected :
			ShaderModule(std::string path, VkShaderStageFlagBits stageBits, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) {
				m_path = path;
				LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logicalDevice = d->getLogicalDevice();
				if (!GetBinaryFileContents(path, m_spirv)) {
					ErrorCheck::setError((char*)"Can't read the Shader file ");
				}
				
				
				if (!CreateShaderModule(logicalDevice, m_spirv, m_module)) {
					ErrorCheck::setError((char*)"Can't create the Shader module");
				}
				
				m_stageParameter = {
						stageBits,
						m_module,
						entrypoint,
						specialization
				};
			}

			ShaderModule(std::vector<unsigned char>	spirv, VkShaderStageFlagBits stageBits, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) {
				m_path = "";
				m_spirv = spirv;
				LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logicalDevice = d->getLogicalDevice();

				if (!CreateShaderModule(logicalDevice, m_spirv, m_module)) {
					ErrorCheck::setError((char*)"Can't create the Shader module");
				}

				m_stageParameter = {
						stageBits,
						m_module,
						entrypoint,
						specialization
				};
			}
      public :
			ShaderStageParameters&	getStageParameter() {
				return m_stageParameter;
			}

			void refresh() {
				LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logicalDevice = d->getLogicalDevice();
				if (!GetBinaryFileContents(m_path, m_spirv)) {
					ErrorCheck::setError((char*)"Can't read the Shader file ");
				}

				if (!CreateShaderModule(logicalDevice, m_spirv, m_module)) {
					ErrorCheck::setError((char*)"Can't create the Shader module");
				}

				m_stageParameter.ShaderModule = m_module;
			}

			~ShaderModule() {
				m_path = "";
				m_spirv.clear();
				m_stageParameter = {};
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				DestroyShaderModule(logical, m_module);
			};

		private :

			bool CreateShaderModule(VkDevice                           logical_device,
				std::vector<unsigned char> const& source_code,
				VkShaderModule& shader_module) {
				VkShaderModuleCreateInfo shader_module_create_info = {
					VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,              // VkStructureType              sType
					nullptr,                                                  // const void                 * pNext
					0,                                                        // VkShaderModuleCreateFlags    flags
					source_code.size(),                                       // size_t                       codeSize
					reinterpret_cast<uint32_t const*>(source_code.data())    // const uint32_t             * pCode
				};

				VkResult result = vkCreateShaderModule(logical_device, &shader_module_create_info, nullptr, &shader_module);
				if (VK_SUCCESS != result) {
					std::cout << "Could not create a shader module." << std::endl;
					return false;
				}
				return true;
			}

			void DestroyShaderModule(VkDevice         logical_device,
				VkShaderModule& shader_module) {
				if (VK_NULL_HANDLE != shader_module) {
					vkDestroyShaderModule(logical_device, shader_module, nullptr);
					shader_module = VK_NULL_HANDLE;
				}
			}


			VkShaderModule								            m_module = VK_NULL_HANDLE;
			std::string																m_path;
			std::vector<unsigned char>								m_spirv;
			ShaderStageParameters											m_stageParameter;
		};

    /**
     Class VertexShaderModule :
     \brief This class helps manage vertex shader module
     */
		class VertexShaderModule : public ShaderModule {
		public:
			VertexShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr):
				ShaderModule(path, VK_SHADER_STAGE_VERTEX_BIT, entrypoint, specialization) {

			}
			VertexShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_VERTEX_BIT, entrypoint, specialization) {
			}
		};
  
    /**
     Class TessellationControlShaderModule :
     \brief This class helps manage tessellation control shader module
     */
		class TessellationControlShaderModule : public ShaderModule {
		public:
			TessellationControlShaderModule( std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule( path, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, entrypoint, specialization) {
			}
			TessellationControlShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, entrypoint, specialization) {
			}
		};

    /**
     Class TessellationEvaluationShaderModule :
     \brief This class helps manage tessellation evaluation shader module
     */
		class TessellationEvaluationShaderModule : public ShaderModule {
		public:
			TessellationEvaluationShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, entrypoint, specialization) {
			}
			TessellationEvaluationShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, entrypoint, specialization) {
			}
		};

    /**
     Class GeometryShaderModule :
     \brief This class helps manage geometry shader module
     */
		class GeometryShaderModule : public ShaderModule {
		public:
			GeometryShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_GEOMETRY_BIT, entrypoint, specialization) {
			}
			GeometryShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_GEOMETRY_BIT, entrypoint, specialization) {
			}
		};

    /**
     Class GeometryShaderModule :
     \brief This class helps manage fragment shader module
     */
		class FragmentShaderModule : public ShaderModule {
		public:
			FragmentShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_FRAGMENT_BIT, entrypoint, specialization) {
			}
			FragmentShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_FRAGMENT_BIT, entrypoint, specialization) {
			}

		};

    /**
     Class GeometryShaderModule :
     \brief This class helps manage compute shader module
     */
		class ComputeShaderModule : public ShaderModule {
		public:
			ComputeShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_COMPUTE_BIT, entrypoint, specialization) {
			}

			ComputeShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_COMPUTE_BIT, entrypoint, specialization) {
			}
		};

		
	}
}
