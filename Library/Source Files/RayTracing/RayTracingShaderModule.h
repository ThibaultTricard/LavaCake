#ifdef RAYTRACING
#pragma once
#include "Framework/ShaderModule.h"



namespace LavaCake {
	namespace RayTracing {

		class RayGenShaderModule : public Framework::ShaderModule {
		public:
			RayGenShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_RAYGEN_BIT_KHR, entrypoint, specialization) {
			}

			RayGenShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_RAYGEN_BIT_KHR, entrypoint, specialization) {
			}
		};

		class AnyHitShaderModule : public Framework::ShaderModule {
		public:
			AnyHitShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, entrypoint, specialization) {
			}

			AnyHitShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, entrypoint, specialization) {
			}
		};


		class ClosestHitShaderModule : public Framework::ShaderModule {
		public:
			ClosestHitShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, entrypoint, specialization) {
			}

			ClosestHitShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, entrypoint, specialization) {
			}
		};

		class MissShaderModule : public Framework::ShaderModule {
		public:
			MissShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_MISS_BIT_KHR, entrypoint, specialization) {
			}

			MissShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_MISS_BIT_KHR, entrypoint, specialization) {
			}
		};

		class IntersectionShaderModule : public Framework::ShaderModule {
		public:
			IntersectionShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, entrypoint, specialization) {
			}

			IntersectionShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, entrypoint, specialization) {
			}
		};

		class CallableShaderModule : public Framework::ShaderModule {
		public:
			CallableShaderModule(std::string path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_CALLABLE_BIT_KHR, entrypoint, specialization) {
			}

			CallableShaderModule(std::vector<unsigned char>	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_CALLABLE_BIT_KHR, entrypoint, specialization) {
			}
		};
	}
}
#endif