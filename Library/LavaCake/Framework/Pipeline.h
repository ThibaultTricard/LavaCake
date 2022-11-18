#include "ShaderModule.h"
#include "VertexBuffer.h"
#include "DescriptorSet.h"


#pragma once


namespace LavaCake {
  namespace Framework {

    struct constant {
      PushConstant* constant;
      VkShaderStageFlags      stage;
    };

    /**
     Class Pipeline :
     \brief A generic class to help manage VkPipelines, inherited by the classes ComputePipeline and GraphicPipeline
     */
    class Pipeline {
    public:

      virtual void setDescriptorLayout(VkDescriptorSetLayout layout){
        m_descriptorSetLayout = layout;
      }

      /*const std::vector<attachment>& getAttachments() {
        if (!m_descriptorSet) {
          m_descriptorSet = std::make_shared< DescriptorSet >();
          ErrorCheck::setError("No Descripor was provided for this pipeline, a default one will be used instead", 1);
        }

        return  m_descriptorSet->getAttachments();
      };*/

      virtual void bindPipeline(CommandBuffer& cmdBuff){};

      virtual void bindDescriptorSet(CommandBuffer& cmdBuffer, const DescriptorSet& descriptorSet){};

      virtual ~Pipeline() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        if (VK_NULL_HANDLE != m_pipeline) {
          vkDestroyPipeline(logical, m_pipeline, nullptr);
          m_pipeline = VK_NULL_HANDLE;
        }

        if (VK_NULL_HANDLE != m_pipelineLayout) {
          vkDestroyPipelineLayout(logical, m_pipelineLayout, nullptr);
          m_pipelineLayout = VK_NULL_HANDLE;
        }

      };

    const VkPipeline& getHandle(){
      return m_pipeline;
    }

    const VkPipelineLayout& getPipelineLayout(){
      return m_pipelineLayout;
    }

    protected:

      void SpecifyPipelineShaderStages(std::vector<Framework::shaderStageParameters> const& shader_stage_params,
        std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_create_infos);


      bool CreatePipelineLayout(VkDevice                                   logical_device,
        std::vector<VkDescriptorSetLayout> const& descriptor_set_layouts,
        std::vector<VkPushConstantRange> const& push_constant_ranges,
        VkPipelineLayout& pipeline_layout);

      bool CreateGraphicsPipelines(VkDevice                                             logical_device,
        std::vector<VkGraphicsPipelineCreateInfo> const& graphics_pipeline_create_infos,
        VkPipelineCache                                      pipeline_cache,
        std::vector<VkPipeline>& graphics_pipelines);


      VkPipeline																					            m_pipeline = VK_NULL_HANDLE;
      VkPipelineLayout																		            m_pipelineLayout = VK_NULL_HANDLE;

      VkDescriptorSetLayout                                           m_descriptorSetLayout;
    };
  }
}
