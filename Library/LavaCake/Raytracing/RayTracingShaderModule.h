#pragma once
#include <LavaCake/Framework/ShaderModule.h>



namespace LavaCake {
	namespace RayTracing {

		class RayGenShaderModule : public Framework::ShaderModule {
		public:

			/**
			\brief Constructor for the RayGenShaderModule
			\param path the path of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			RayGenShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_RAYGEN_BIT_KHR, entrypoint, specialization) {
			}

			/**
			\brief Constructor for the RayGenShaderModule
			\param spirv a vector of char encoding the spirv data of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			RayGenShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_RAYGEN_BIT_KHR, entrypoint, specialization) {
			}
		};

		class AnyHitShaderModule : public Framework::ShaderModule {
		public:

			/**
			\brief Constructor for the AnyHitShaderModule
			\param path the path of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			AnyHitShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, entrypoint, specialization) {
			}

			/**
			\brief Constructor for the AnyHitShaderModule
			\param spirv a vector of char encoding the spirv data of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			AnyHitShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, entrypoint, specialization) {
			}
		};


		class ClosestHitShaderModule : public Framework::ShaderModule {
		public:

			/**
			\brief Constructor for the ClosestHitShaderModule
			\param path the path of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			ClosestHitShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, entrypoint, specialization) {
			}

			/**
			\brief Constructor for the ClosestHitShaderModule
			\param spirv a vector of char encoding the spirv data of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			ClosestHitShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, entrypoint, specialization) {
			}
		};

		class MissShaderModule : public Framework::ShaderModule {
		public:

			/**
			\brief Constructor for the MissShaderModule
			\param path the path of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			MissShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_MISS_BIT_KHR, entrypoint, specialization) {
			}

			/**
			\brief Constructor for the MissShaderModule
			\param spirv a vector of char encoding the spirv data of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			MissShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_MISS_BIT_KHR, entrypoint, specialization) {
			}
		};

		class IntersectionShaderModule : public Framework::ShaderModule {
		public:

			/**
			\brief Constructor for the IntersectionShaderModule
			\param path the path of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			IntersectionShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, entrypoint, specialization) {
			}


			/**
			\brief Constructor for the IntersectionShaderModule
			\param spirv a vector of char encoding the spirv data of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			IntersectionShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, entrypoint, specialization) {
			}
		};

		class CallableShaderModule : public Framework::ShaderModule {
		public:

			/**
			\brief Constructor for the CallableShaderModule
			\param path the path of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			CallableShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_CALLABLE_BIT_KHR, entrypoint, specialization) {
			}


			/**
			\brief Constructor for the CallableShaderModule
			\param spirv a vector of char encoding the spirv data of the shader
			\param entrypoint the entry point of the shader
			\param specialization a VkSpecializationInfo instance
			*/
			CallableShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_CALLABLE_BIT_KHR, entrypoint, specialization) {
			}
		};
	}
}