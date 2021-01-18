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

		void Pipeline::SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription> const& binding_descriptions,
			std::vector<VkVertexInputAttributeDescription> const& attribute_descriptions,
			VkPipelineVertexInputStateCreateInfo& vertex_input_state_create_info) {
			vertex_input_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,    // VkStructureType                           sType
				nullptr,                                                      // const void                              * pNext
				0,                                                            // VkPipelineVertexInputStateCreateFlags     flags
				static_cast<uint32_t>(binding_descriptions.size()),           // uint32_t                                  vertexBindingDescriptionCount
				binding_descriptions.data(),                                  // const VkVertexInputBindingDescription   * pVertexBindingDescriptions
				static_cast<uint32_t>(attribute_descriptions.size()),         // uint32_t                                  vertexAttributeDescriptionCount
				attribute_descriptions.data()                                 // const VkVertexInputAttributeDescription * pVertexAttributeDescriptions
			};
		}

		void Pipeline::SpecifyPipelineInputAssemblyState(VkPrimitiveTopology                      topology,
			bool                                     primitive_restart_enable,
			VkPipelineInputAssemblyStateCreateInfo& input_assembly_state_create_info) {
			input_assembly_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                           sType
				nullptr,                                                      // const void                              * pNext
				0,                                                            // VkPipelineInputAssemblyStateCreateFlags   flags
				topology,                                                     // VkPrimitiveTopology                       topology
				primitive_restart_enable                                      // VkBool32                                  primitiveRestartEnable
			};
		}

		void Pipeline::SpecifyPipelineTessellationState(uint32_t                                patch_control_points_count,
			VkPipelineTessellationStateCreateInfo& tessellation_state_create_info) {
			tessellation_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,    // VkStructureType                            sType
				nullptr,                                                      // const void                               * pNext
				0,                                                            // VkPipelineTessellationStateCreateFlags     flags
				patch_control_points_count                                    // uint32_t                                   patchControlPoints
			};
		}



		void Pipeline::SpecifyPipelineRasterizationState(bool                                     depth_clamp_enable,
			bool                                     rasterizer_discard_enable,
			VkPolygonMode                            polygon_mode,
			VkCullModeFlags                          culling_mode,
			VkFrontFace                              front_face,
			bool                                     depth_bias_enable,
			float                                    depth_bias_constant_factor,
			float                                    depth_bias_clamp,
			float                                    depth_bias_slope_factor,
			float                                    line_width,
			VkPipelineRasterizationStateCreateInfo& rasterization_state_create_info) {
			rasterization_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType                            sType
				nullptr,                                                    // const void                               * pNext
				0,                                                          // VkPipelineRasterizationStateCreateFlags    flags
				depth_clamp_enable,                                         // VkBool32                                   depthClampEnable
				rasterizer_discard_enable,                                  // VkBool32                                   rasterizerDiscardEnable
				polygon_mode,                                               // VkPolygonMode                              polygonMode
				culling_mode,                                               // VkCullModeFlags                            cullMode
				front_face,                                                 // VkFrontFace                                frontFace
				depth_bias_enable,                                          // VkBool32                                   depthBiasEnable
				depth_bias_constant_factor,                                 // float                                      depthBiasConstantFactor
				depth_bias_clamp,                                           // float                                      depthBiasClamp
				depth_bias_slope_factor,                                    // float                                      depthBiasSlopeFactor
				line_width                                                  // float                                      lineWidth
			};
		}

		void Pipeline::SpecifyPipelineMultisampleState(VkSampleCountFlagBits                  sample_count,
			bool                                   per_sample_shading_enable,
			float                                  min_sample_shading,
			VkSampleMask const* sample_masks,
			bool                                   alpha_to_coverage_enable,
			bool                                   alpha_to_one_enable,
			VkPipelineMultisampleStateCreateInfo& multisample_state_create_info) {
			multisample_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, // VkStructureType                          sType
				nullptr,                                                  // const void                             * pNext
				0,                                                        // VkPipelineMultisampleStateCreateFlags    flags
				sample_count,                                             // VkSampleCountFlagBits                    rasterizationSamples
				per_sample_shading_enable,                                // VkBool32                                 sampleShadingEnable
				min_sample_shading,                                       // float                                    minSampleShading
				sample_masks,                                             // const VkSampleMask                     * pSampleMask
				alpha_to_coverage_enable,                                 // VkBool32                                 alphaToCoverageEnable
				alpha_to_one_enable                                       // VkBool32                                 alphaToOneEnable
			};
		}

		void Pipeline::SpecifyPipelineDepthAndStencilState(bool                                    depth_test_enable,
			bool                                    depth_write_enable,
			VkCompareOp                             depth_compare_op,
			bool                                    depth_bounds_test_enable,
			float                                   min_depth_bounds,
			float                                   max_depth_bounds,
			bool                                    stencil_test_enable,
			VkStencilOpState                        front_stencil_test_parameters,
			VkStencilOpState                        back_stencil_test_parameters,
			VkPipelineDepthStencilStateCreateInfo& depth_and_stencil_state_create_info) {
			depth_and_stencil_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,   // VkStructureType                            sType
				nullptr,                                                      // const void                               * pNext
				0,                                                            // VkPipelineDepthStencilStateCreateFlags     flags
				depth_test_enable,                                            // VkBool32                                   depthTestEnable
				depth_write_enable,                                           // VkBool32                                   depthWriteEnable
				depth_compare_op,                                             // VkCompareOp                                depthCompareOp
				depth_bounds_test_enable,                                     // VkBool32                                   depthBoundsTestEnable
				stencil_test_enable,                                          // VkBool32                                   stencilTestEnable
				front_stencil_test_parameters,                                // VkStencilOpState                           front
				back_stencil_test_parameters,                                 // VkStencilOpState                           back
				min_depth_bounds,                                             // float                                      minDepthBounds
				max_depth_bounds                                              // float                                      maxDepthBounds
			};
		}

		void Pipeline::SpecifyPipelineBlendState(bool                                                     logic_op_enable,
			VkLogicOp                                                logic_op,
			std::vector<VkPipelineColorBlendAttachmentState> const& attachment_blend_states,
			std::array<float, 4> const& blend_constants,
			VkPipelineColorBlendStateCreateInfo& blend_state_create_info) {
			blend_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // VkStructureType                              sType
				nullptr,                                                    // const void                                 * pNext
				0,                                                          // VkPipelineColorBlendStateCreateFlags         flags
				logic_op_enable,                                            // VkBool32                                     logicOpEnable
				logic_op,                                                   // VkLogicOp                                    logicOp
				static_cast<uint32_t>(attachment_blend_states.size()),      // uint32_t                                     attachmentCount
				attachment_blend_states.data(),                             // const VkPipelineColorBlendAttachmentState  * pAttachments
				{                                                           // float                                        blendConstants[4]
					blend_constants[0],
					blend_constants[1],
					blend_constants[2],
					blend_constants[3]
				}
			};
		}


		void Pipeline::SpecifyPipelineDynamicStates(std::vector<VkDynamicState> const& dynamic_states,
			VkPipelineDynamicStateCreateInfo& dynamic_state_creat_info) {
			dynamic_state_creat_info = {
				VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,     // VkStructureType                      sType
				nullptr,                                                  // const void                         * pNext
				0,                                                        // VkPipelineDynamicStateCreateFlags    flags
				static_cast<uint32_t>(dynamic_states.size()),             // uint32_t                             dynamicStateCount
				dynamic_states.data()                                     // const VkDynamicState               * pDynamicStates
			};
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
				std::cout << "Could not create pipeline layout." << std::endl;
				return false;
			}
			return true;
		}

		void Pipeline::SpecifyGraphicsPipelineCreationParameters(VkPipelineCreateFlags                                additional_options,
			std::vector<VkPipelineShaderStageCreateInfo> const& shader_stage_create_infos,
			VkPipelineVertexInputStateCreateInfo const& vertex_input_state_create_info,
			VkPipelineInputAssemblyStateCreateInfo const& input_assembly_state_create_info,
			VkPipelineTessellationStateCreateInfo const* tessellation_state_create_info,
			VkPipelineViewportStateCreateInfo const* viewport_state_create_info,
			VkPipelineRasterizationStateCreateInfo const& rasterization_state_create_info,
			VkPipelineMultisampleStateCreateInfo const* multisample_state_create_info,
			VkPipelineDepthStencilStateCreateInfo const* depth_and_stencil_state_create_info,
			VkPipelineColorBlendStateCreateInfo const* blend_state_create_info,
			VkPipelineDynamicStateCreateInfo const* dynamic_state_creat_info,
			VkPipelineLayout                                     pipeline_layout,
			VkRenderPass                                         render_pass,
			uint32_t                                             subpass,
			VkPipeline                                           base_pipeline_handle,
			int32_t                                              base_pipeline_index,
			VkGraphicsPipelineCreateInfo& graphics_pipeline_create_info) {
			graphics_pipeline_create_info = {
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                sType
				nullptr,                                                    // const void                                   * pNext
				additional_options,                                         // VkPipelineCreateFlags                          flags
				static_cast<uint32_t>(shader_stage_create_infos.size()),    // uint32_t                                       stageCount
				shader_stage_create_infos.data(),                           // const VkPipelineShaderStageCreateInfo        * pStages
				& vertex_input_state_create_info,                            // const VkPipelineVertexInputStateCreateInfo   * pVertexInputState
				& input_assembly_state_create_info,                          // const VkPipelineInputAssemblyStateCreateInfo * pInputAssemblyState
				tessellation_state_create_info,                             // const VkPipelineTessellationStateCreateInfo  * pTessellationState
				viewport_state_create_info,                                 // const VkPipelineViewportStateCreateInfo      * pViewportState
				& rasterization_state_create_info,                           // const VkPipelineRasterizationStateCreateInfo * pRasterizationState
				multisample_state_create_info,                              // const VkPipelineMultisampleStateCreateInfo   * pMultisampleState
				depth_and_stencil_state_create_info,                        // const VkPipelineDepthStencilStateCreateInfo  * pDepthStencilState
				blend_state_create_info,                                    // const VkPipelineColorBlendStateCreateInfo    * pColorBlendState
				dynamic_state_creat_info,                                   // const VkPipelineDynamicStateCreateInfo       * pDynamicState
				pipeline_layout,                                            // VkPipelineLayout                               layout
				render_pass,                                                // VkRenderPass                                   renderPass
				subpass,                                                    // uint32_t                                       subpass
				base_pipeline_handle,                                       // VkPipeline                                     basePipelineHandle
				base_pipeline_index                                         // int32_t                                        basePipelineIndex
			};
		}

		bool Pipeline::CreatePipelineCacheObject(VkDevice                           logical_device,
			std::vector<unsigned char> const& cache_data,
			VkPipelineCache& pipeline_cache) {
			VkPipelineCacheCreateInfo pipeline_cache_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,     // VkStructureType                sType
				nullptr,                                          // const void                   * pNext
				0,                                                // VkPipelineCacheCreateFlags     flags
				static_cast<uint32_t>(cache_data.size()),         // size_t                         initialDataSize
				cache_data.data()                                 // const void                   * pInitialData
			};

			VkResult result = vkCreatePipelineCache(logical_device, &pipeline_cache_create_info, nullptr, &pipeline_cache);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create pipeline cache." << std::endl;
				return false;
			}
			return true;
		}

		bool Pipeline::RetrieveDataFromPipelineCache(VkDevice                     logical_device,
			VkPipelineCache              pipeline_cache,
			std::vector<unsigned char>& pipeline_cache_data) {
			size_t data_size = 0;
			VkResult result = VK_SUCCESS;

			result = vkGetPipelineCacheData(logical_device, pipeline_cache, &data_size, nullptr);
			if ((VK_SUCCESS != result) ||
				(0 == data_size)) {
				std::cout << "Could not get the size of the pipeline cache." << std::endl;
				return false;
			}
			pipeline_cache_data.resize(data_size);

			result = vkGetPipelineCacheData(logical_device, pipeline_cache, &data_size, pipeline_cache_data.data());
			if ((VK_SUCCESS != result) ||
				(0 == data_size)) {
				std::cout << "Could not acquire pipeline cache data." << std::endl;
				return false;
			}

			return true;
		}

		bool Pipeline::MergeMultiplePipelineCacheObjects(VkDevice                             logical_device,
			VkPipelineCache                      target_pipeline_cache,
			std::vector<VkPipelineCache> const& source_pipeline_caches) {
			if (source_pipeline_caches.size() > 0) {
				VkResult result = vkMergePipelineCaches(logical_device, target_pipeline_cache, static_cast<uint32_t>(source_pipeline_caches.size()), source_pipeline_caches.data());
				if (VK_SUCCESS != result) {
					std::cout << "Could not merge pipeline cache objects." << std::endl;
					return false;
				}
				return true;
			}
			return false;
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

		bool Pipeline::CreateComputePipeline(VkDevice                                logical_device,
			VkPipelineCreateFlags                   additional_options,
			VkPipelineShaderStageCreateInfo const& compute_shader_stage,
			VkPipelineLayout                        pipeline_layout,
			VkPipeline                              base_pipeline_handle,
			VkPipelineCache                         pipeline_cache,
			VkPipeline& compute_pipeline) {
			VkComputePipelineCreateInfo compute_pipeline_create_info = {
				VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,   // VkStructureType                    sType
				nullptr,                                          // const void                       * pNext
				additional_options,                               // VkPipelineCreateFlags              flags
				compute_shader_stage,                             // VkPipelineShaderStageCreateInfo    stage
				pipeline_layout,                                  // VkPipelineLayout                   layout
				base_pipeline_handle,                             // VkPipeline                         basePipelineHandle
				-1                                                // int32_t                            basePipelineIndex
			};

			VkResult result = vkCreateComputePipelines(logical_device, pipeline_cache, 1, &compute_pipeline_create_info, nullptr, &compute_pipeline);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create compute pipeline." << std::endl;
				return false;
			}
			return true;
		}

		void Pipeline::BindPipelineObject(VkCommandBuffer     command_buffer,
			VkPipelineBindPoint pipeline_type,
			VkPipeline          pipeline) {
			vkCmdBindPipeline(command_buffer, pipeline_type, pipeline);
		}

		void Pipeline::DispatchComputeWork(VkCommandBuffer command_buffer,
			uint32_t        x_size,
			uint32_t        y_size,
			uint32_t        z_size) {
			vkCmdDispatch(command_buffer, x_size, y_size, z_size);
		}


		void Pipeline::DestroyPipeline(VkDevice     logical_device,
			VkPipeline& pipeline) {
			if (VK_NULL_HANDLE != pipeline) {
				vkDestroyPipeline(logical_device, pipeline, nullptr);
				pipeline = VK_NULL_HANDLE;
			}
		}

		void Pipeline::DestroyPipelineCache(VkDevice          logical_device,
			VkPipelineCache& pipeline_cache) {
			if (VK_NULL_HANDLE != pipeline_cache) {
				vkDestroyPipelineCache(logical_device, pipeline_cache, nullptr);
				pipeline_cache = VK_NULL_HANDLE;
			}
		}

		void Pipeline::DestroyPipelineLayout(VkDevice           logical_device,
			VkPipelineLayout& pipeline_layout) {
			if (VK_NULL_HANDLE != pipeline_layout) {
				vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
				pipeline_layout = VK_NULL_HANDLE;
			}
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
				ErrorCheck::setError("Can't create descriptor set layout");
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
				ErrorCheck::setError("Can't create descriptor pool");
			}

			if (!LavaCake::Core::AllocateDescriptorSets(logical, *m_descriptorPool, { *m_descriptorSetLayout }, m_descriptorSets)) {
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
								m_attachments[i].a->getImageView(),					// VkImageView                          offset
								m_attachments[i].a->getLayout()							// VkImageLayout                         range
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
