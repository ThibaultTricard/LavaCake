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
					shader_stage.ShaderStage,                             // VkShaderStageFlagBits              stage
					shader_stage.ShaderModule,                            // VkShaderModule                     module
					shader_stage.EntryPointName,                          // const char                       * pName
					shader_stage.SpecializationInfo                       // const VkSpecializationInfo       * pSpecializationInfo
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
					std::cout << "Could not create a graphics pipeline." << std::endl;
					return false;
				}
				return true;
			}
			return false;
		}

		void Pipeline::generateDescriptorLayout() {

			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();

			m_descriptorSetLayoutBinding = {};

			for (uint32_t i = 0; i < m_uniforms.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_uniforms[i].binding),
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					1,
					m_uniforms[i].stage,
					nullptr
					});
			}
			for (uint32_t i = 0; i < m_textures.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_textures[i].binding),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					m_textures[i].stage,
					nullptr
					});
			}
			for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_frameBuffers[i].binding),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					m_frameBuffers[i].stage,
					nullptr
					});
			}
			for (uint32_t i = 0; i < m_attachments.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_attachments[i].binding),
					VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
					1,
					m_attachments[i].stage,
					nullptr
					});
			}
			for (uint32_t i = 0; i < m_storageImages.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_storageImages[i].binding),
					VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
					1,
					m_storageImages[i].stage,
					nullptr
					});
			}
			for (uint32_t i = 0; i < m_texelBuffers.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_texelBuffers[i].binding),
					VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
					1,
					m_texelBuffers[i].stage,
					nullptr
					});
			}

			InitVkDestroyer(logical, m_descriptorSetLayout);
			if (!LavaCake::Core::CreateDescriptorSetLayout(logical, m_descriptorSetLayoutBinding, *m_descriptorSetLayout)) {
				ErrorCheck::setError((char*)"Can't create descriptor set layout");
			}

			m_descriptorCount = static_cast<uint32_t>(m_uniforms.size() + m_textures.size() + m_storageImages.size() + m_attachments.size() + m_frameBuffers.size() + m_texelBuffers.size());
			if (m_descriptorCount == 0) return;

			m_descriptorPoolSize = {};

			if (m_uniforms.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						uint32_t(m_uniforms.size())
					});
			}
			if (m_textures.size() + m_frameBuffers.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						uint32_t(m_textures.size() + m_frameBuffers.size())
					});
			}
			if (m_attachments.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
						uint32_t(m_attachments.size())
					});
			}
			if (m_storageImages.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
						uint32_t(m_storageImages.size())
					});
			}
			if (m_texelBuffers.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
						uint32_t(m_texelBuffers.size())
					});
			}

			InitVkDestroyer(logical, m_descriptorPool);
			if (!LavaCake::Core::CreateDescriptorPool(logical, false, m_descriptorCount, m_descriptorPoolSize, *m_descriptorPool)) {
				ErrorCheck::setError((char*)"Can't create descriptor pool");
			}

			if (!LavaCake::Core::AllocateDescriptorSets(logical, *m_descriptorPool, { *m_descriptorSetLayout }, m_descriptorSets)) {
				ErrorCheck::setError((char*)"Can't allocate descriptor set");
			}
			m_bufferDescriptorUpdate = { };
			int descriptorCount = 0;
			for (uint32_t i = 0; i < m_uniforms.size(); i++) {
				m_bufferDescriptorUpdate.push_back({
					m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
					uint32_t(m_uniforms[i].binding),								// uint32_t                             TargetDescriptorBinding
					0,																							// uint32_t                             TargetArrayElement
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,							// VkDescriptorType                     TargetDescriptorType
					{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
						{
							m_uniforms[i].buffer->getHandle(),					// VkBuffer                             buffer
							0,																					// VkDeviceSize                         offset
							VK_WHOLE_SIZE																// VkDeviceSize                         range
						}
					}
					});
			}

			m_imageDescriptorUpdate = { };
			for (uint32_t i = 0; i < m_textures.size(); i++) {
				m_imageDescriptorUpdate.push_back({
					m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
					uint32_t(m_textures[i].binding),								// uint32_t                             TargetDescriptorBinding
					0,																							// uint32_t                             TargetArrayElement
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,			// VkDescriptorType                     TargetDescriptorType
					{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
						{
							m_textures[i].t->getSampler(),							// vkSampler                            buffer
							m_textures[i].t->getImageView(),						// VkImageView                          offset
							m_textures[i].t->getLayout()								// VkImageLayout                         range
						}
					}
					});
			}
			for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
				std::vector<VkDescriptorImageInfo> descriptorInfo;
				descriptorInfo.push_back(
					{
						m_frameBuffers[i].f->getSampler(),
						m_frameBuffers[i].f->getImageViews(m_frameBuffers[i].viewIndex),
						m_frameBuffers[i].f->getLayout(m_frameBuffers[i].viewIndex),
					});

				m_imageDescriptorUpdate.push_back({
					m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
					uint32_t(m_frameBuffers[i].binding),						// uint32_t                             TargetDescriptorBinding
					0,																							// uint32_t                             TargetArrayElement
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,			// VkDescriptorType                     TargetDescriptorType
					descriptorInfo																	// std::vector<VkDescriptorBufferInfo>  BufferInfos
					});
			}
			for (uint32_t i = 0; i < m_attachments.size(); i++) {
				m_imageDescriptorUpdate.push_back({
						m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
						uint32_t(m_attachments[i].binding),								// uint32_t                             TargetDescriptorBinding
						0,																							// uint32_t                             TargetArrayElement
						VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,						// VkDescriptorType                     TargetDescriptorType
						{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
							{
								VK_NULL_HANDLE,															// vkSampler                            buffer
								m_attachments[i].a->getImage()->getImageView(),					// VkImageView                          offset
								m_attachments[i].a->getImage()->getLayout()							// VkImageLayout                         range
							}
						}
					});
			}

			for (uint32_t i = 0; i < m_storageImages.size(); i++) {
				m_imageDescriptorUpdate.push_back({
						m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
						uint32_t(m_storageImages[i].binding),						// uint32_t                             TargetDescriptorBinding
						0,																							// uint32_t                             TargetArrayElement
						VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,								// VkDescriptorType                     TargetDescriptorType
						{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
							{
								VK_NULL_HANDLE,															// vkSampler                            buffer
								m_storageImages[i].s->getImageView(),				// VkImageView                          offset
								m_storageImages[i].s->getLayout()						// VkImageLayout                         range
							}
						}
					});
			}

			m_texelBufferDescriptorUpdate = {};
			for (uint32_t i = 0; i < m_texelBuffers.size(); i++) {

				LavaCake::Core::TexelBufferDescriptorInfo info = {
					m_descriptorSets[descriptorCount],
					uint32_t(m_texelBuffers[i].binding),
					0,
					VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
					{
						m_texelBuffers[i].t->getBufferView()
					}
				};

				m_texelBufferDescriptorUpdate.push_back(info);
			}
			/////////////////////////////////////////////////////////////////////////////////////////////
			LavaCake::Core::UpdateDescriptorSets(logical, m_imageDescriptorUpdate, m_bufferDescriptorUpdate, { m_texelBufferDescriptorUpdate }, {});
		}
	}
}
