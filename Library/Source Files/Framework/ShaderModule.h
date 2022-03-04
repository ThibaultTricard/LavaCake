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
			ShaderModule(const std::string& path, VkShaderStageFlagBits stageBits, char const* entrypoint = "main", VkSpecializationInfo const * specialization = nullptr) {
				m_path = path;
				std::vector<unsigned char> spriv;
				if (!GetBinaryFileContents(path, spriv)) {
					ErrorCheck::setError("Can't read the Shader file.");
				}
				
				if (!createShaderModule(spriv)) {
					ErrorCheck::setError("Can't create the Shader module.");
				}
				
				m_stageParameter = {
						stageBits,
						m_module,
						entrypoint,
						specialization
				};
			}

			ShaderModule(const std::vector<unsigned char>&	spirv, VkShaderStageFlagBits stageBits, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) {
				m_path = "";
				
				if (!createShaderModule(spirv)) {
					ErrorCheck::setError("Can't create the Shader module.");
				}

				m_stageParameter = {
						stageBits,
						m_module,
						entrypoint,
						specialization
				};
			}

			ShaderModule(ShaderModule&) = delete;

			ShaderModule(ShaderModule&& s) {
				m_module = s.m_module;
				m_path = s.m_path;
				m_stageParameter = s.m_stageParameter;
				s.m_module = VK_NULL_HANDLE;
			}

      public :
			const ShaderStageParameters&	getStageParameter() const{
				return m_stageParameter;
			}

			~ShaderModule() {
				m_path = "";
				m_stageParameter = {};
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				if (VK_NULL_HANDLE != m_module) {
					vkDestroyShaderModule(logical, m_module, nullptr);
					m_module = VK_NULL_HANDLE;
				}
			};

		private :

			bool createShaderModule(const std::vector<unsigned char>& src) {
				VkShaderModuleCreateInfo createInfo = {
					VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,              // VkStructureType              sType
					nullptr,                                                  // const void                 * pNext
					0,                                                        // VkShaderModuleCreateFlags    flags
					src.size(),																								// size_t                       codeSize
					reinterpret_cast<uint32_t const*>(src.data())							// const uint32_t             * pCode
				};
				VkDevice logical = Device::getDevice()->getLogicalDevice();
				VkResult result = vkCreateShaderModule(logical, &createInfo, nullptr, &m_module);
				if (VK_SUCCESS != result) {
					ErrorCheck::setError("Could not create a shader module.");
					return false;
				}
				return true;
			}

      void destroyShaderModule(VkDevice /*logical_device*/,
                               VkShaderModule& /*shader_module*/) {
			}

			VkShaderModule								            m_module = VK_NULL_HANDLE;
			std::string																m_path;
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

		/**
		 Class MeshShaderModule :
		 \brief This class helps manage mesh shader module
		 */
		class MeshShaderModule : public ShaderModule {
		public:
			MeshShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_MESH_BIT_NV, entrypoint, specialization) {
			}

			MeshShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_MESH_BIT_NV, entrypoint, specialization) {
			}
		};


		/**
		 Class TaskShaderModule :
		 \brief This class helps manage mesh shader module
		 */
		class TaskShaderModule : public ShaderModule {
		public:
			TaskShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_TASK_BIT_NV, entrypoint, specialization) {
			}

			TaskShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_TASK_BIT_NV, entrypoint, specialization) {
			}
		};
		
	}
}
