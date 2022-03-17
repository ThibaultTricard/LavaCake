#pragma once
#include "Framework/ShaderModule.h"



namespace LavaCake {
	namespace RayTracing {

		class RayGenShaderModule : public Framework::ShaderModule {
		public:
			RayGenShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_RAYGEN_BIT_KHR, entrypoint, specialization) {
			}

			RayGenShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_RAYGEN_BIT_KHR, entrypoint, specialization) {
			}
		};

		class AnyHitShaderModule : public Framework::ShaderModule {
		public:
			AnyHitShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, entrypoint, specialization) {
			}

			AnyHitShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_ANY_HIT_BIT_KHR, entrypoint, specialization) {
			}
		};


		class ClosestHitShaderModule : public Framework::ShaderModule {
		public:
			ClosestHitShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, entrypoint, specialization) {
			}

			ClosestHitShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, entrypoint, specialization) {
			}
		};

		class MissShaderModule : public Framework::ShaderModule {
		public:
			MissShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_MISS_BIT_KHR, entrypoint, specialization) {
			}

			MissShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_MISS_BIT_KHR, entrypoint, specialization) {
			}
		};

		class IntersectionShaderModule : public Framework::ShaderModule {
		public:
			IntersectionShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, entrypoint, specialization) {
			}

			IntersectionShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, entrypoint, specialization) {
			}
		};

		class CallableShaderModule : public Framework::ShaderModule {
		public:
			CallableShaderModule(const std::string& path, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(path, VK_SHADER_STAGE_CALLABLE_BIT_KHR, entrypoint, specialization) {
			}

			CallableShaderModule(const std::vector<unsigned char>&	spirv, char const* entrypoint = "main", VkSpecializationInfo const* specialization = nullptr) :
				ShaderModule(spirv, VK_SHADER_STAGE_CALLABLE_BIT_KHR, entrypoint, specialization) {
			}
		};
	}
}