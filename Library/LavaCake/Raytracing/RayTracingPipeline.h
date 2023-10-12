#pragma once
#include <LavaCake/Raytracing/RayTracingShaderModule.h>
#include <LavaCake/Framework/CommandBuffer.h>
#include <LavaCake/Framework/Pipeline.h>
#include <LavaCake/Framework/ErrorCheck.h>
#include "ShaderBindingTable.h"
#include "TopLevelAS.h"

namespace LavaCake {
  namespace RayTracing {



    class RayTracingPipeline : public Framework::Pipeline {
    public:

		RayTracingPipeline() : Pipeline() {};


		void addRayGenModule(const RayGenShaderModule& module);

		void addMissModule(const MissShaderModule& module);

		void startHitGroup();

		void setClosestHitModule(const ClosestHitShaderModule& module);

		void setAnyHitModule(const AnyHitShaderModule& module);

		void setIntersectionModule(const IntersectionShaderModule& module);

		void endHitGroup();

		void compile(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff);

		void trace(Framework::CommandBuffer& cmdbuff, uint32_t dimX, uint32_t dimY = 1.0, uint32_t dimZ = 1.0);

		void setMaxRecursion(uint16_t recursion = 1);

		void bindDescriptorSet( Framework::CommandBuffer& cmdBuffer, const  Framework::DescriptorSet& descriptorSet) override;

      void bindPipeline( Framework::CommandBuffer& cmdBuff) override;

    private:


			ShaderBindingTable m_ShaderBindingTable;
			
			std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups;

			VkPhysicalDeviceRayTracingPipelinePropertiesKHR  m_rayTracingPipelineProperties{};

			bool m_isHitGroupOpen = false;
			uint32_t m_currentGroupIndex = 0;

			std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

			uint16_t m_maxRecursion = 1;
    };
  }
}
