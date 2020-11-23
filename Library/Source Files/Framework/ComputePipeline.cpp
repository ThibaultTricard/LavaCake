#include "ComputePipeline.h"

namespace LavaCake {
	namespace Framework {

		void ComputePipeline::compile() {
			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			generateDescriptorLayout();
			InitVkDestroyer(logical, m_pipelineLayout);
			if (!CreatePipelineLayout(logical, { *m_descriptorSetLayout }, {}, *m_pipelineLayout)) {
				ErrorCheck::setError("Can't create compute pipeline layout");
			}

			std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
			SpecifyPipelineShaderStages({ m_computeModule->getStageParameter() }, shader_stage_create_infos);

			InitVkDestroyer(logical, m_pipeline);
			if (!CreateComputePipeline(logical, 0, shader_stage_create_infos[0], *m_pipelineLayout, VK_NULL_HANDLE, VK_NULL_HANDLE, *m_pipeline)) {
				ErrorCheck::setError("Can't create compute pipeline");
			}
		}

		void ComputePipeline::setComputeShader(ComputeShaderModule*	module) {
			m_computeModule = module;
		}

		void ComputePipeline::compute(const VkCommandBuffer buffer, uint32_t dimX, uint32_t dimY, uint32_t dimZ) {
			LavaCake::Core::BindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, *m_pipelineLayout, 0, m_descriptorSets, {});

			Pipeline::BindPipelineObject(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, *m_pipeline);

			Pipeline::DispatchComputeWork(buffer, dimX, dimY, dimZ);
		}


		
	}
}