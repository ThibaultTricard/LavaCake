#include "GraphicPipeline.h"

namespace LavaCake {
	namespace Framework {

		GraphicPipeline::GraphicPipeline(vec3f viewportMin, vec3f viewportMax, vec2f scisorMin, vec2f scisorMax) {
			//viewport
			m_viewports = {
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
			};

			m_scissors = {
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

			m_viewportInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,    // VkStructureType                      sType
				nullptr,                                                  // const void                         * pNext
				0,                                                        // VkPipelineViewportStateCreateFlags   flags
				1,																												// uint32_t                             viewportCount
				m_viewports.data(),																				// const VkViewport                   * pViewports
				1,																												// uint32_t                             scissorCount
				m_scissors.data()																					// const VkRect2D                     * pScissors
			};

		};

		void GraphicPipeline::setVertexModule(VertexShaderModule*	module) {
			if (m_type == pipelineType::Undefined || m_type == pipelineType::Graphic) {
				m_vertexModule = module;
				m_type = pipelineType::Graphic;
			}
			else if(m_type == pipelineType::MeshTask) {
				ErrorCheck::setError("This pipeline is allready setup to use Mesh and Task shader module, it cannot use a vertex shader module",1);
			}
		}

		void GraphicPipeline::setTesselationControlModule(TessellationControlShaderModule*	module) {
			if (m_type == pipelineType::Undefined || m_type == pipelineType::Graphic) {
				m_tesselationControlModule = module;
				m_type = pipelineType::Graphic;
			}
			else if (m_type == pipelineType::MeshTask) {
				ErrorCheck::setError("This pipeline is allready setup to use Mesh and Task shader module, it cannot use a tesselation control module", 1);
			}
		}

		void GraphicPipeline::setTesselationEvaluationModule(TessellationEvaluationShaderModule*	module) {
			if (m_type == pipelineType::Undefined || m_type == pipelineType::Graphic) {
				m_tesselationEvaluationModule = module;
				m_type = pipelineType::Graphic;
			}
			else if (m_type == pipelineType::MeshTask) {
				ErrorCheck::setError("This pipeline is allready setup to use Mesh and Task shader module, it cannot use a tesselation evaluation shader module", 1);
			}
		}

		void GraphicPipeline::setGeometryModule(GeometryShaderModule*	module) {
			if (m_type == pipelineType::Undefined || m_type == pipelineType::Graphic) {
				m_geometryModule = module;
				m_type = pipelineType::Graphic;
			}
			else if (m_type == pipelineType::MeshTask) {
				ErrorCheck::setError("This pipeline is allready setup to use Mesh and Task shader module. It cannot use a geometry shader module", 1);
			}

		}

		void GraphicPipeline::setFragmentModule(FragmentShaderModule*	module) {
			m_fragmentModule = module;
		}

		void GraphicPipeline::setMeshModule(MeshShaderModule* module) {
			if (m_type == pipelineType::Undefined || m_type == pipelineType::MeshTask) {
				m_meshModule = module;
				m_type = pipelineType::MeshTask;
			}
			else if (m_type == pipelineType::Graphic) {
				ErrorCheck::setError("This pipeline is allready setup to use vertex , tesselation control, tesselation evaluation, or geometry shader module.  It cannot use a mesh shader module", 1);
			}
		}

		void GraphicPipeline::setTaskModule(TaskShaderModule* module) {
			if (m_type == pipelineType::Undefined || m_type == pipelineType::MeshTask) {
				m_taskModule = module;
				m_type = pipelineType::MeshTask;
			}
			else if (m_type == pipelineType::Graphic) {
				ErrorCheck::setError("This pipeline is allready setup to use vertex , tesselation control, tesselation evaluation, or geometry shader module.  It cannot use a task shader module", 1);
			}
		}


		std::vector<ShaderStageParameters> GraphicPipeline::getStageParameter() {
			std::vector<ShaderStageParameters> stage = std::vector<ShaderStageParameters>();
			if (m_taskModule != nullptr) {
				stage.push_back(m_taskModule->getStageParameter());
			}
			if (m_meshModule != nullptr) {
				stage.push_back(m_meshModule->getStageParameter());
			}
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

			std::vector<VkPushConstantRange> push_constant_ranges = {};
			for (uint32_t i = 0; i < m_constants.size(); i++) {
                const uint32_t size =(const uint32_t) m_constants[i].constant->size() * sizeof(float);
				push_constant_ranges.push_back(
					{
						m_constants[i].stage,																				// VkShaderStageFlags     stageFlags
						0,																													// uint32_t               offset
                        size// uint32_t               size
					});
			}

			if (!Pipeline::CreatePipelineLayout(logical, { m_descriptorSet->getLayout() }, push_constant_ranges, m_pipelineLayout)) {
				ErrorCheck::setError("Can't create pipeline layout");
			}

			
			m_rasterizationStateCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType                            sType
				nullptr,                                                    // const void                               * pNext
				0,                                                          // VkPipelineRasterizationStateCreateFlags    flags
				false,																											// VkBool32                                   depthClampEnable
				false,																											// VkBool32                                   rasterizerDiscardEnable
				m_polygonMode,																							// VkPolygonMode                              polygonMode
				VkCullModeFlags(m_cullMode),																// VkCullModeFlags                            cullMode
				VK_FRONT_FACE_COUNTER_CLOCKWISE,                            // VkFrontFace                                frontFace
				false,																											// VkBool32                                   depthBiasEnable
				0.0f,																												// float                                      depthBiasConstantFactor
				0.0f,																												// float                                      depthBiasClamp
				0.0f,																												// float                                      depthBiasSlopeFactor
        m_lineWidth																									// float                                      lineWidth
			};

			m_multisampleStateCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, // VkStructureType                          sType
				nullptr,                                                  // const void                             * pNext
				0,                                                        // VkPipelineMultisampleStateCreateFlags    flags
				VK_SAMPLE_COUNT_1_BIT,                                    // VkSampleCountFlagBits                    rasterizationSamples
				false,																										// VkBool32                                 sampleShadingEnable
				0.0f,																											// float                                    minSampleShading
				nullptr,																								  // const VkSampleMask                     * pSampleMask
				false,																										// VkBool32                                 alphaToCoverageEnable
				false																											// VkBool32                                 alphaToOneEnable
			};

			m_depthStencilStateCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,   // VkStructureType                            sType
				nullptr,                                                      // const void                               * pNext
				0,                                                            // VkPipelineDepthStencilStateCreateFlags     flags
				true,																													// VkBool32                                   depthTestEnable
				true,																													// VkBool32                                   depthWriteEnable
				VK_COMPARE_OP_LESS_OR_EQUAL,                                  // VkCompareOp                                depthCompareOp
				false,																												// VkBool32                                   depthBoundsTestEnable
				false,																												// VkBool32                                   stencilTestEnable
				{},																														// VkStencilOpState                           front
				{},																														// VkStencilOpState                           back
				0.0f,																													// float                                      minDepthBounds
				1.0f																													// float                                      maxDepthBounds
			};

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

			
			m_blendStateCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // VkStructureType                              sType
				nullptr,                                                    // const void                                 * pNext
				0,                                                          // VkPipelineColorBlendStateCreateFlags         flags
				false,																								      // VkBool32                                     logicOpEnable
				VK_LOGIC_OP_COPY,                                           // VkLogicOp                                    logicOp
				static_cast<uint32_t>(m_attachmentBlendStates.size()),      // uint32_t                                     attachmentCount
				m_attachmentBlendStates.data(),                             // const VkPipelineColorBlendAttachmentState  * pAttachments
				{                                                           // float                                        blendConstants[4]
					1.0f,
					1.0f,
					1.0f,
					1.0f
				}
			};

			m_dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			if (m_lineWidth != 1.0f) {
				m_dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
			}

			m_dynamicStateCreateInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,     // VkStructureType                      sType
				nullptr,                                                  // const void                         * pNext
				0,                                                        // VkPipelineDynamicStateCreateFlags    flags
				static_cast<uint32_t>(m_dynamicStates.size()),             // uint32_t                             dynamicStateCount
				m_dynamicStates.data()                                     // const VkDynamicState               * pDynamicStates
			};

			Pipeline::SpecifyPipelineShaderStages(getStageParameter(), m_shaderStageCreateInfos);

			m_pipelineCreateInfo = {
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                sType
				nullptr,                                                    // const void                                   * pNext
				0,																													// VkPipelineCreateFlags                          flags
				static_cast<uint32_t>(m_shaderStageCreateInfos.size()),			// uint32_t                                       stageCount
				m_shaderStageCreateInfos.data(),														// const VkPipelineShaderStageCreateInfo        * pStages
				& m_vertexInfo,																							// const VkPipelineVertexInputStateCreateInfo   * pVertexInputState
				& m_inputInfo,																							// const VkPipelineInputAssemblyStateCreateInfo * pInputAssemblyState
				nullptr,																										// const VkPipelineTessellationStateCreateInfo  * pTessellationState
				& m_viewportInfo,																						// const VkPipelineViewportStateCreateInfo      * pViewportState
				& m_rasterizationStateCreateInfo,                           // const VkPipelineRasterizationStateCreateInfo * pRasterizationState
				& m_multisampleStateCreateInfo,                             // const VkPipelineMultisampleStateCreateInfo   * pMultisampleState
				& m_depthStencilStateCreateInfo,														// const VkPipelineDepthStencilStateCreateInfo  * pDepthStencilState
				& m_blendStateCreateInfo,                                   // const VkPipelineColorBlendStateCreateInfo    * pColorBlendState
				& m_dynamicStateCreateInfo,                                 // const VkPipelineDynamicStateCreateInfo       * pDynamicState
        m_pipelineLayout,                                         // VkPipelineLayout                               layout
				renderpass,																									// VkRenderPass                                   renderPass
				m_subpassNumber,                                            // uint32_t                                       subpass
				VK_NULL_HANDLE,																							// VkPipeline                                     basePipelineHandle
				-1																													// int32_t                                        basePipelineIndex
			};

			std::vector<VkPipeline> pipelines;
			if (!Pipeline::CreateGraphicsPipelines(logical, { m_pipelineCreateInfo }, VK_NULL_HANDLE, pipelines)) {
				ErrorCheck::setError("Can't create Graphics piepeline");
			}
			m_pipeline = pipelines[0];
			m_compiled = true;
		}

		void GraphicPipeline::recompile() {
			Device* d = Device::getDevice();
			VkDevice& logical = d->getLogicalDevice();

			std::vector<VkPipeline> pipelines;
			if (!Pipeline::CreateGraphicsPipelines(logical, { m_pipelineCreateInfo }, VK_NULL_HANDLE, pipelines)) {
				ErrorCheck::setError("Can't create Graphics piepeline");
			}
			m_pipeline = pipelines[0];
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
			if (m_meshModule != nullptr) {
				m_meshModule->refresh();
			}
			if (m_taskModule != nullptr) {
				m_taskModule->refresh();
			}
		}

		void GraphicPipeline::setSubpassNumber(uint32_t number) {
			m_subpassNumber = number;
		}

		void GraphicPipeline::setVerticesInfo(std::vector<VkVertexInputBindingDescription>& bindingDescription,
																					std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, 
																					VkPrimitiveTopology topology) {
			m_vertexInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,								// VkStructureType                           sType
				nullptr,																																	// const void                              * pNext
				0,																																				// VkPipelineVertexInputStateCreateFlags     flags
				static_cast<uint32_t>(bindingDescription.size()),           // uint32_t                                  vertexBindingDescriptionCount
				bindingDescription.data(),                                  // const VkVertexInputBindingDescription   * pVertexBindingDescriptions
				static_cast<uint32_t>(attributeDescriptions.size()),         // uint32_t                                  vertexAttributeDescriptionCount
				attributeDescriptions.data()                                 // const VkVertexInputAttributeDescription * pVertexAttributeDescriptions
			};

			m_inputInfo = {
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                           sType
				nullptr,                                                      // const void                              * pNext
				0,                                                            // VkPipelineInputAssemblyStateCreateFlags   flags
				topology,																											// VkPrimitiveTopology                       topology
				false																													// VkBool32                                  primitiveRestartEnable
			};

			if (m_compiled) {
				m_pipelineCreateInfo.pVertexInputState = &m_vertexInfo;
				m_pipelineCreateInfo.pInputAssemblyState = &m_inputInfo;
				recompile();
			}
		}

		void GraphicPipeline::setVertices(std::vector<VertexBuffer*> buffer) {
			m_vertexBuffers = buffer;
		}



		void GraphicPipeline::draw(CommandBuffer& buffer) {
			vkCmdSetViewport(buffer.getHandle(), 0, 1,  &m_viewports[0] );

			vkCmdSetScissor(buffer.getHandle(), 0, 1,  &m_scissors[0]);

			if (m_lineWidth != 1.0) {
				vkCmdSetLineWidth(buffer.getHandle(), m_lineWidth);
			}

			for (uint32_t i = 0; i < m_vertexBuffers.size(); i++) {
				if (m_vertexBuffers[i]->getVertexBuffer()->getHandle() == VK_NULL_HANDLE)return;
				VkDeviceSize size(0);
				vkCmdBindVertexBuffers(buffer.getHandle(), 0, static_cast<uint32_t>(1), &m_vertexBuffers[i]->getVertexBuffer()->getHandle(), &size);
				if (m_vertexBuffers[i]->isIndexed()) {
					vkCmdBindIndexBuffer(buffer.getHandle(), m_vertexBuffers[i]->getIndexBuffer()->getHandle(), VkDeviceSize(0), VK_INDEX_TYPE_UINT32);
				}
        
        std::vector<VkDescriptorSet> descriptorSets = {m_descriptorSet->getHandle()};
				if (!m_descriptorSet->isEmpty()) {
					vkCmdBindDescriptorSets(buffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
						static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
						0, {});
				}

				vkCmdBindPipeline(buffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

				for (uint32_t i = 0; i < m_constants.size(); i++) {
					m_constants[i].constant->push(buffer.getHandle(), m_pipelineLayout, m_constants[i].stage);
				}

				if (m_type == pipelineType::Graphic) {
					if (m_vertexBuffers[i]->isIndexed()) {
						uint32_t count = (uint32_t)m_vertexBuffers[i]->getIndicesNumber();

						vkCmdDrawIndexed(buffer.getHandle(), count, 1, 0, 0, 0);
					}
					else {

						uint32_t count = (uint32_t)m_vertexBuffers[i]->getVerticiesNumber();

						vkCmdDraw(buffer.getHandle(), count, 1, 0, 0);
					}
				}
				else if (m_type == pipelineType::MeshTask) {
					vkCmdDrawMeshTasksNV(
						buffer.getHandle(),
						m_taskCount,
						0);
				}

			}
		}

		void GraphicPipeline::SetCullMode(VkCullModeFlagBits cullMode) {
			m_cullMode = cullMode;
		}

		void GraphicPipeline::addPushContant(PushConstant* constant, VkShaderStageFlags flag) {
			m_constants.push_back({ constant, flag });
		}

		
	}
}
