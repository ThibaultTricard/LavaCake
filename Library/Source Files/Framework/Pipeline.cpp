#include "Pipeline.h"
namespace LavaCake {
	namespace Framework {

		void Pipeline::SpecifyPipelineShaderStages(std::vector<ShaderStageParameters> const& shader_stage_params,
			std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_create_infos) {
			shader_stage_create_infos.clear();
			for (auto& shader_stage : shader_stage_params) {
				shader_stage_create_infos.push_back({
					VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // VkStructureType                    sType
					nullptr,                                              // const void                       * pNext
					0,                                                    // VkPipelineShaderStageCreateFlags   flags
					shader_stage.shaderStage,                             // VkShaderStageFlagBits              stage
					shader_stage.shaderModule,                            // VkShaderModule                     module
					shader_stage.entryPointName,                          // const char                       * pName
					shader_stage.specializationInfo                       // const VkSpecializationInfo       * pSpecializationInfo
					});
			}
		}


		bool Pipeline::CreatePipelineLayout(VkDevice                                   logical_device,
			std::vector<VkDescriptorSetLayout> const& descriptor_set_layouts,
			std::vector<VkPushConstantRange> const& push_constant_ranges,
			VkPipelineLayout& pipeline_layout) {
			VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,          // VkStructureType                  sType
				nullptr,                                                // const void                     * pNext
				0,                                                      // VkPipelineLayoutCreateFlags      flags
				static_cast<uint32_t>(descriptor_set_layouts.size()),   // uint32_t                         setLayoutCount
				descriptor_set_layouts.data(),                          // const VkDescriptorSetLayout    * pSetLayouts
				static_cast<uint32_t>(push_constant_ranges.size()),     // uint32_t                         pushConstantRangeCount
				push_constant_ranges.data()                             // const VkPushConstantRange      * pPushConstantRanges
			};

			VkResult result = vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout);

			if (VK_SUCCESS != result) {
				return false;
			}
			return true;
		}

		bool Pipeline::CreateGraphicsPipelines(VkDevice                                             logical_device,
			std::vector<VkGraphicsPipelineCreateInfo> const& graphics_pipeline_create_infos,
			VkPipelineCache                                      pipeline_cache,
			std::vector<VkPipeline>& graphics_pipelines) {
			if (graphics_pipeline_create_infos.size() > 0) {
				graphics_pipelines.resize(graphics_pipeline_create_infos.size());
				VkResult result = vkCreateGraphicsPipelines(logical_device, pipeline_cache, static_cast<uint32_t>(graphics_pipeline_create_infos.size()), graphics_pipeline_create_infos.data(), nullptr, graphics_pipelines.data());
				if (VK_SUCCESS != result) {
					return false;
				}
				return true;
			}
			return false;
		}

		void Pipeline::generateDescriptorLayout() {
      if(!m_descriptorSet){
        m_descriptorSet = std::make_shared < DescriptorSet >();
        ErrorCheck::setError("No Descripor was provided for this pipeline, a default one will be used instead", 1);
      }
      m_descriptorSet->generateDescriptorLayout();
    }
	}
}
