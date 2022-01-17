#pragma once
#include "RayTracingShaderModule.h"
#include "Framework/CommandBuffer.h"
#include "Framework/Pipeline.h"
#include "Framework/ErrorCheck.h"
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


			void addRayGenModule(RayGenShaderModule* module);

			void addMissModule(MissShaderModule* module);

			void startHitGroup();

			void setClosestHitModule(ClosestHitShaderModule* module);

			void setAnyHitModule(AnyHitShaderModule* module);

			void setIntersectionModule(IntersectionShaderModule* module);

			void endHitGroup();

			void compile(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff);

			void trace(Framework::CommandBuffer& cmdbuff);

			void setMaxRecursion(uint16_t recursion = 1);

			void addAccelerationStructure(TopLevelAccelerationStructure* AS, VkShaderStageFlags stage, uint32_t	binding) {
				if (m_descriptorSet == nullptr) {
					m_descriptorSet = new LavaCake::Framework::DescriptorSet();
					m_isDescriptorPipelineGenerated = true;
				}
				if (m_isDescriptorPipelineGenerated) {
					m_descriptorSet->addAccelerationStructure(AS, stage, binding);
				}
				else {
					LavaCake::Framework::ErrorCheck::setError((char*)"Descritpor is not generated by the pipeline, modifying it this way could affect other pipeline");
				}
				
			}

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
