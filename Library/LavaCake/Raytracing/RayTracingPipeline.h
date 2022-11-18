#pragma once
#include "RayTracingShaderModule.h"
#include <LavaCake/Framework/CommandBuffer.h>
#include <LavaCake/Framework/Pipeline.h>
#include <LavaCake/Framework/ErrorCheck.h>
#include "ShaderBindingTable.h"
#include "TopLevelAS.h"

namespace LavaCake {
  namespace RayTracing {



    class RayTracingPipeline : public Framework::Pipeline {
    public:

			RayTracingPipeline(vec2u viewPortSize) : Pipeline() {
				m_width = viewPortSize[0];
				m_height = viewPortSize[1];
			};


			void addRayGenModule(const RayGenShaderModule& module);

			void addMissModule(const MissShaderModule& module);

			void startHitGroup();

			void setClosestHitModule(const ClosestHitShaderModule& module);

			void setAnyHitModule(const AnyHitShaderModule& module);

			void setIntersectionModule(const IntersectionShaderModule& module);

			void endHitGroup();

			void compile(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff);

			void trace(Framework::CommandBuffer& cmdbuff);

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
			

			uint32_t m_width;
			uint32_t m_height;

			uint16_t m_maxRecursion = 1;
    };
  }
}
