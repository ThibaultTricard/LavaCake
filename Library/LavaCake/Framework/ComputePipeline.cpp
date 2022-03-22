#include "ComputePipeline.h"

namespace LavaCake {
  namespace Framework {

    void ComputePipeline::compile() {
      Device* d = Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      generateDescriptorLayout();
      if (!CreatePipelineLayout(logical, { m_descriptorSet->getLayout() }, {}, m_pipelineLayout)) {
        ErrorCheck::setError("Can't create compute pipeline layout");
      }

      std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
      SpecifyPipelineShaderStages({ m_computeModule }, shader_stage_create_infos);


      VkComputePipelineCreateInfo compute_pipeline_create_info = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,   // VkStructureType                    sType
        nullptr,                                          // const void                       * pNext
        0,																								// VkPipelineCreateFlags              flags
        shader_stage_create_infos[0],                     // VkPipelineShaderStageCreateInfo    stage
        m_pipelineLayout,																// VkPipelineLayout                   layout
        VK_NULL_HANDLE,																		// VkPipeline                         basePipelineHandle
        -1                                                // int32_t                            basePipelineIndex
      };

      VkResult result = vkCreateComputePipelines(logical, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &m_pipeline);
      if (VK_SUCCESS != result) {
        ErrorCheck::setError("Can't create compute pipeline");
      }
    }

    void ComputePipeline::setComputeModule(const ComputeShaderModule& module) {
      m_computeModule = module.getStageParameter();
    }

    void ComputePipeline::compute(CommandBuffer& buffer, uint32_t dimX, uint32_t dimY, uint32_t dimZ) {

      if (!m_descriptorSet->isEmpty()) {
        vkCmdBindDescriptorSets(buffer.getHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0,
          static_cast<uint32_t>(1), &m_descriptorSet->getHandle(),
          0, {});
      }

      vkCmdBindPipeline(buffer.getHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

      vkCmdDispatch(buffer.getHandle(), dimX, dimY, dimZ);

    }



  }
}
