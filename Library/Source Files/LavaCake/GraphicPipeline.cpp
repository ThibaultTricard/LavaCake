#include "LavaCake/GraphicPipeline.h"

namespace LavaCake {
	namespace Framework {

		GraphicPipeline::GraphicPipeline(vec3f viewportMin, vec3f viewportMax, vec2f scisorMin, vec2f scisorMax) {
			//viewport
			m_viewportscissor = {
					{                     // std::vector<VkViewport>   Viewports
						{
							viewportMin[0],																// float          x
							viewportMin[1],																// float          y
							viewportMax[0] - viewportMin[0],							// float          width
							viewportMax[1] - viewportMin[1],							// float          height
							viewportMin[2],																// float          minDepth
							viewportMax[2]																// float          maxDepth
						}
					},
					{                     // std::vector<VkRect2D>     Scissors
						{
							{                   // VkOffset2D     offset
								int32_t(scisorMin[0]),															 // int32_t        x
								int32_t(scisorMin[1])																 // int32_t        y
							},
							{                   // VkExtent2D     extent
								uint32_t(scisorMax[0] - scisorMin[0]),                // uint32_t       width
								uint32_t(scisorMax[1] - scisorMin[1])								  // uint32_t       height
							}
						}
					}
			};
			Viewport::SpecifyPipelineViewportAndScissorTestState(m_viewportscissor, m_viewportInfo);
		};

		void GraphicPipeline::setVextexShader(VertexShaderModule*	module) {
			m_vertexModule = module;
		}

		void GraphicPipeline::setTesselationControlModule(TessellationControlShaderModule*	module) {
			m_tesselationControlModule = module;
		}

		void GraphicPipeline::setTesselationEvaluationModule(TessellationEvaluationShaderModule*	module) {
			m_tesselationEvaluationModule = module;
		}

		void GraphicPipeline::setGeometryModule(GeometryShaderModule*	module) {
			m_geometryModule = module;
		}

		void GraphicPipeline::setFragmentModule(FragmentShaderModule*	module) {
			m_fragmentModule = module;
		}

		std::vector<Shader::ShaderStageParameters> GraphicPipeline::getStageParameter() {
			std::vector<Shader::ShaderStageParameters> stage = std::vector<Shader::ShaderStageParameters>();
			if (m_vertexModule != nullptr) {
				stage.push_back(m_vertexModule->getStageParameter());
			}
			if (m_tesselationControlModule != nullptr) {
				stage.push_back(m_tesselationControlModule->getStageParameter());
			}
			if (m_tesselationEvaluationModule != nullptr) {
				stage.push_back(m_tesselationEvaluationModule->getStageParameter());
			}
			if (m_geometryModule != nullptr) {
				stage.push_back(m_geometryModule->getStageParameter());
			}
			if (m_fragmentModule != nullptr) {
				stage.push_back(m_fragmentModule->getStageParameter());
			}
			return stage;
		}

		void GraphicPipeline::compile(VkRenderPass& renderpass) {
			Device* d = Device::getDevice();
			VkDevice& logical = d->getLogicalDevice();
			generateDescriptorLayout();
			InitVkDestroyer(logical, m_pipelineLayout);

			std::vector<VkPushConstantRange> push_constant_ranges = {};
			for (uint32_t i = 0; i < m_constants.size(); i++) {
				push_constant_ranges.push_back(
					{
						m_constants[i].stage,																				// VkShaderStageFlags     stageFlags
						0,																													// uint32_t               offset
						m_constants[i].constant->size() * sizeof(float)		          // uint32_t               size
					});
			}

			if (!Pipeline::CreatePipelineLayout(logical, { *m_descriptorSetLayout }, push_constant_ranges, *m_pipelineLayout)) {
				ErrorCheck::setError("Can't create pipeline layout");
			}

			Pipeline::SpecifyPipelineRasterizationState(false, false, VK_POLYGON_MODE_FILL, m_CullMode, VK_FRONT_FACE_COUNTER_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f, m_rasterizationStateCreateInfo);
			Pipeline::SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, nullptr, false, false, m_multisampleStateCreateInfo);
			Pipeline::SpecifyPipelineDepthAndStencilState(true, true, VK_COMPARE_OP_LESS_OR_EQUAL, false, 0.0f, 1.0f, false, {}, {}, m_depthStencilStateCreateInfo);
			m_attachmentBlendStates = {
				{
					false,                          // VkBool32                 blendEnable
					VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcColorBlendFactor
					VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstColorBlendFactor
					VK_BLEND_OP_ADD,                // VkBlendOp                colorBlendOp
					VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcAlphaBlendFactor
					VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstAlphaBlendFactor
					VK_BLEND_OP_ADD,                // VkBlendOp                alphaBlendOp
					VK_COLOR_COMPONENT_R_BIT |      // VkColorComponentFlags    colorWriteMask
					VK_COLOR_COMPONENT_G_BIT |
					VK_COLOR_COMPONENT_B_BIT |
					VK_COLOR_COMPONENT_A_BIT
				}
			};
			Pipeline::SpecifyPipelineBlendState(false, VK_LOGIC_OP_COPY, m_attachmentBlendStates, { 1.0f, 1.0f, 1.0f, 1.0f }, m_blendStateCreateInfo);
			m_dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			Pipeline::SpecifyPipelineDynamicStates(m_dynamicStates, m_dynamicStateCreateInfo);
			Pipeline::SpecifyPipelineShaderStages(getStageParameter(), m_shaderStageCreateInfos);
			Pipeline::SpecifyGraphicsPipelineCreationParameters(0, m_shaderStageCreateInfos, m_vertexInfo, m_inputInfo,
				nullptr, &m_viewportInfo, m_rasterizationStateCreateInfo, &m_multisampleStateCreateInfo, &m_depthStencilStateCreateInfo, &m_blendStateCreateInfo,
				&m_dynamicStateCreateInfo, *m_pipelineLayout, renderpass, m_subpassNumber, VK_NULL_HANDLE, -1, m_pipelineCreateInfo);

			std::vector<VkPipeline> pipelines;
			if (!Pipeline::CreateGraphicsPipelines(logical, { m_pipelineCreateInfo }, VK_NULL_HANDLE, pipelines)) {
				ErrorCheck::setError("Can't create Graphics piepeline");
			}
			InitVkDestroyer(logical, m_pipeline);
			*m_pipeline = pipelines[0];
		}

		void GraphicPipeline::addUniformBuffer(UniformBuffer * b, VkShaderStageFlags stage, int binding) {
			m_uniforms.push_back({ b,binding,stage });
		}


		void GraphicPipeline::addTextureBuffer(TextureBuffer * t, VkShaderStageFlags stage, int binding) {
			m_textures.push_back({ t,binding,stage });
		}

		void GraphicPipeline::addStorageImage(StorageImage * s, VkShaderStageFlags stage, int binding) {
			m_storageImage.push_back({ s,binding,stage });
		}

		void GraphicPipeline::addAttachment(Attachment * a, VkShaderStageFlags stage, int binding) {
			m_attachments.push_back({ a,binding,stage });
		}

		void GraphicPipeline::addFrameBuffer(FrameBuffer * f, VkShaderStageFlags stage, int binding) {
			m_frameBuffer.push_back({ f, binding, stage });
		}

		void GraphicPipeline::setSubpassNumber(uint32_t number) {
			m_subpassNumber = number;
		}

		void GraphicPipeline::setVeritices(VertexBuffer* buffer) {
			m_vertexBuffer = buffer;
			Pipeline::SpecifyPipelineVertexInputState(buffer->getBindingDescriptions(), buffer->getAttributeDescriptions(), m_vertexInfo);

			Pipeline::SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false, m_inputInfo);
		}

		void GraphicPipeline::draw(const VkCommandBuffer buffer) {
			VkViewport& viewport = m_viewportscissor.Viewports[0];
			Viewport::SetViewportStateDynamically(buffer, 0, { viewport });

			VkRect2D& scissor = m_viewportscissor.Scissors[0];
			Viewport::SetScissorStateDynamically(buffer, 0, { scissor });

			Buffer::BindVertexBuffers(buffer, 0, { { m_vertexBuffer->getBuffer(), 0 } });
			if (m_descriptorCount > 0) {
				Descriptor::BindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipelineLayout, 0, m_descriptorSets, {});
			}

			Pipeline::BindPipelineObject(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipeline);

			for (uint32_t i = 0; i < m_constants.size(); i++) {
				m_constants[i].constant->push(buffer, *m_pipelineLayout, m_constants[i].stage);
			}

			uint32_t count = 0;
			for (size_t i = 0; i < m_vertexBuffer->getMeshs().size(); i++) {
				for (size_t j = 0; j < m_vertexBuffer->getMeshs()[i]->Parts.size(); j++) {
					count += m_vertexBuffer->getMeshs()[i]->Parts[j].VertexCount;
				}
			}
			LavaCake::vkCmdDraw(buffer, count, 1, 0, 0);
		}

		void GraphicPipeline::SetCullMode(VkCullModeFlagBits cullMode) {
			m_CullMode = cullMode;
		}

		void GraphicPipeline::addPushContant(PushConstant* constant, VkShaderStageFlags flag) {
			m_constants.push_back({ constant, flag });
		}

		void GraphicPipeline::generateDescriptorLayout() {

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
			for (uint32_t i = 0; i < m_frameBuffer.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_frameBuffer[i].binding),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					m_frameBuffer[i].stage,
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
			for (uint32_t i = 0; i < m_storageImage.size(); i++) {
				m_descriptorSetLayoutBinding.push_back({
					uint32_t(m_storageImage[i].binding),
					VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
					1,
					m_attachments[i].stage,
					nullptr
					});
			}

			InitVkDestroyer(logical, m_descriptorSetLayout);
			if (!LavaCake::Descriptor::CreateDescriptorSetLayout(logical, m_descriptorSetLayoutBinding, *m_descriptorSetLayout)) {
				ErrorCheck::setError("Can't create descriptor set layout");
			}

			m_descriptorCount = m_uniforms.size() + m_textures.size() + m_storageImage.size() + m_attachments.size() + m_frameBuffer.size();
			if (m_descriptorCount == 0) return;

			m_descriptorPoolSize = {};

			if (m_uniforms.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,							
						uint32_t(m_uniforms.size())										
					});
			}
			if (m_textures.size() + m_frameBuffer.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,			
						uint32_t(m_textures.size()+m_frameBuffer.size())										
					});
			}
			if (m_attachments.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,						
						uint32_t(m_attachments.size())								
					});
			}
			if (m_storageImage.size() > 0) {
				m_descriptorPoolSize.push_back({
					VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,								// VkDescriptorType     type
						uint32_t(m_storageImage.size())								// uint32_t             descriptorCount
					});
			}

			
			InitVkDestroyer(logical, m_descriptorPool);
			if (!LavaCake::Descriptor::CreateDescriptorPool(logical, false, m_descriptorCount, m_descriptorPoolSize, *m_descriptorPool)) {
				ErrorCheck::setError("Can't create descriptor pool");
			}

			if (!LavaCake::Descriptor::AllocateDescriptorSets(logical, *m_descriptorPool, { *m_descriptorSetLayout }, m_descriptorSets)) {
				ErrorCheck::setError("Can't allocate descriptor set");
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
							m_uniforms[i].buffer->getBuffer(),					// VkBuffer                             buffer
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
			for (uint32_t i = 0; i < m_frameBuffer.size(); i++) {
				std::vector<VkDescriptorImageInfo> descriptorInfo;
				for (int j = 0; j < m_frameBuffer[i].f->ImageViewSize(); j++) {
					descriptorInfo.push_back(
						{
							m_frameBuffer[i].f->getSampler(),
							m_frameBuffer[i].f->getImageViews(j),
							m_frameBuffer[i].f->getLayout(),
						});
				}
				
				m_imageDescriptorUpdate.push_back({
					m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
					uint32_t(m_frameBuffer[i].binding),							// uint32_t                             TargetDescriptorBinding
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
								m_attachments[i].a->getImageView(),					// VkImageView                          offset
								m_attachments[i].a->getLayout()							// VkImageLayout                         range
							}
						}
					});
			}
			for (uint32_t i = 0; i < m_storageImage.size(); i++) {
				m_imageDescriptorUpdate.push_back({
						m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
						uint32_t(m_storageImage[i].binding),						// uint32_t                             TargetDescriptorBinding
						0,																							// uint32_t                             TargetArrayElement
						VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,								// VkDescriptorType                     TargetDescriptorType
						{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
							{
								VK_NULL_HANDLE,															// vkSampler                            buffer
								m_storageImage[i].s->getImageView(),				// VkImageView                          offset
								m_storageImage[i].s->getLayout()						// VkImageLayout                         range
							}
						}
					});
			}
			/////////////////////////////////////////////////////////////////////////////////////////////
			Descriptor::UpdateDescriptorSets(logical, m_imageDescriptorUpdate, m_bufferDescriptorUpdate, {}, {});
		}
	}
}