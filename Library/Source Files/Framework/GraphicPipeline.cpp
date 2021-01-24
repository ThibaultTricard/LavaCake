#include "GraphicPipeline.h"

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
			LavaCake::Core::SpecifyPipelineViewportAndScissorTestState(m_viewportscissor, m_viewportInfo);
		};

		void GraphicPipeline::setVextexModule(VertexShaderModule*	module) {
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

		std::vector<ShaderStageParameters> GraphicPipeline::getStageParameter() {
			std::vector<ShaderStageParameters> stage = std::vector<ShaderStageParameters>();
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

		void GraphicPipeline::compile(VkRenderPass& renderpass, uint16_t nbColorAttachments) {
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

			

			Pipeline::SpecifyPipelineRasterizationState(false, false, m_vertexBuffer->polygonMode(), m_CullMode, VK_FRONT_FACE_COUNTER_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f, m_rasterizationStateCreateInfo);
			Pipeline::SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, nullptr, false, false, m_multisampleStateCreateInfo);
			Pipeline::SpecifyPipelineDepthAndStencilState(true, true, VK_COMPARE_OP_LESS_OR_EQUAL, false, 0.0f, 1.0f, false, {}, {}, m_depthStencilStateCreateInfo);
			
			m_attachmentBlendStates = {};
			for (uint16_t c = 0; c < nbColorAttachments; c++) {
				m_attachmentBlendStates.push_back(
					{
						m_alphablending,																// VkBool32                 blendEnable
						VK_BLEND_FACTOR_SRC_ALPHA,											// VkBlendFactor            srcColorBlendFactor
						VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,            // VkBlendFactor            dstColorBlendFactor
						VK_BLEND_OP_ADD,																// VkBlendOp                colorBlendOp
						VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,						// VkBlendFactor            srcAlphaBlendFactor
						VK_BLEND_FACTOR_ZERO,														// VkBlendFactor            dstAlphaBlendFactor
						VK_BLEND_OP_ADD,																// VkBlendOp                alphaBlendOp
						VK_COLOR_COMPONENT_R_BIT |											// VkColorComponentFlags    colorWriteMask
						VK_COLOR_COMPONENT_G_BIT |
						VK_COLOR_COMPONENT_B_BIT |
						VK_COLOR_COMPONENT_A_BIT
					}
				);
				
			}

			
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
			m_compiled = true;
		}

		void GraphicPipeline::recompile() {
			Device* d = Device::getDevice();
			VkDevice& logical = d->getLogicalDevice();

			std::vector<VkPipeline> pipelines;
			if (!Pipeline::CreateGraphicsPipelines(logical, { m_pipelineCreateInfo }, VK_NULL_HANDLE, pipelines)) {
				ErrorCheck::setError("Can't create Graphics piepeline");
			}
			*m_pipeline = pipelines[0];
		}

		void GraphicPipeline::reloadShaders() {
			if (m_vertexModule != nullptr) {
				m_vertexModule->refresh();
			}
			if (m_tesselationControlModule != nullptr) {
				m_tesselationControlModule->refresh();
			}
			if (m_tesselationEvaluationModule != nullptr) {
				m_tesselationEvaluationModule->refresh();
			}
			if (m_geometryModule != nullptr) {
				m_geometryModule->refresh();
			}
			if (m_fragmentModule != nullptr) {
				m_fragmentModule->refresh();
			}
		}

	

		void GraphicPipeline::setSubpassNumber(uint32_t number) {
			m_subpassNumber = number;
		}

		void GraphicPipeline::setVertices(VertexBuffer* buffer) {
			m_vertexBuffer = buffer;
			Pipeline::SpecifyPipelineVertexInputState(buffer->getBindingDescriptions(), buffer->getAttributeDescriptions(), m_vertexInfo);

			Pipeline::SpecifyPipelineInputAssemblyState(buffer->primitiveTopology(), false, m_inputInfo);
			if (m_compiled) { 
				m_pipelineCreateInfo.pVertexInputState = &m_vertexInfo;
				m_pipelineCreateInfo.pInputAssemblyState = &m_inputInfo;
				recompile(); 
			}
		}



		void GraphicPipeline::draw(const VkCommandBuffer buffer) {
			VkViewport& viewport = m_viewportscissor.Viewports[0];
			LavaCake::Core::SetViewportStateDynamically(buffer, 0, { viewport });

			

			VkRect2D& scissor = m_viewportscissor.Scissors[0];
			LavaCake::Core::SetScissorStateDynamically(buffer, 0, { scissor });
			if (m_vertexBuffer->getVertexBuffer().getHandle() == VK_NULL_HANDLE)return;
			VkDeviceSize size(0);
			vkCmdBindVertexBuffers(buffer, 0, static_cast<uint32_t>(1), { &m_vertexBuffer->getVertexBuffer().getHandle() }, { &size });
			if (m_vertexBuffer->isIndexed()) {
				vkCmdBindIndexBuffer(buffer, m_vertexBuffer->getIndexBuffer().getHandle(), VkDeviceSize(0), VK_INDEX_TYPE_UINT32);
			}

			if (m_descriptorCount > 0) {
				LavaCake::Core::BindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipelineLayout, 0, m_descriptorSets, {});
			}

			Pipeline::BindPipelineObject(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipeline);

			for (uint32_t i = 0; i < m_constants.size(); i++) {
				m_constants[i].constant->push(buffer, *m_pipelineLayout, m_constants[i].stage);
			}

			if (m_vertexBuffer->isIndexed()) {
				uint32_t count = (uint32_t)m_vertexBuffer->getIndicesNumber();
				
				LavaCake::vkCmdDrawIndexed(buffer, count, 1, 0, 0, 0);
			}else{
				
				uint32_t count = (uint32_t)m_vertexBuffer->getVerticiesNumber();

				LavaCake::vkCmdDraw(buffer, count, 1, 0, 0);
			}
			
		}

		void GraphicPipeline::SetCullMode(VkCullModeFlagBits cullMode) {
			m_CullMode = cullMode;
		}

		void GraphicPipeline::addPushContant(PushConstant* constant, VkShaderStageFlags flag) {
			m_constants.push_back({ constant, flag });
		}

		
	}
}