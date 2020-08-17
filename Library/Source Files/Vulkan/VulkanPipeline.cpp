// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and / or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The below copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Vulkan Cookbook
// ISBN: 9781786468154
// © Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329


#include "VulkanPipeline.h"
#include "VulkanShader.h"

namespace LavaCake {
	namespace Pipeline {
		void SpecifyPipelineShaderStages(std::vector<Shader::ShaderStageParameters> const     & shader_stage_params,
			std::vector<VkPipelineShaderStageCreateInfo> & shader_stage_create_infos) {
			shader_stage_create_infos.clear();
			for (auto & shader_stage : shader_stage_params) {
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
	
		void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription> const   & binding_descriptions,
			std::vector<VkVertexInputAttributeDescription> const & attribute_descriptions,
			VkPipelineVertexInputStateCreateInfo                 & vertex_input_state_create_info) {
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

		void SpecifyPipelineInputAssemblyState(VkPrimitiveTopology                      topology,
			bool                                     primitive_restart_enable,
			VkPipelineInputAssemblyStateCreateInfo & input_assembly_state_create_info) {
			input_assembly_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                           sType
				nullptr,                                                      // const void                              * pNext
				0,                                                            // VkPipelineInputAssemblyStateCreateFlags   flags
				topology,                                                     // VkPrimitiveTopology                       topology
				primitive_restart_enable                                      // VkBool32                                  primitiveRestartEnable
			};
		}

		void SpecifyPipelineTessellationState(uint32_t                                patch_control_points_count,
			VkPipelineTessellationStateCreateInfo & tessellation_state_create_info) {
			tessellation_state_create_info = {
				VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,    // VkStructureType                            sType
				nullptr,                                                      // const void                               * pNext
				0,                                                            // VkPipelineTessellationStateCreateFlags     flags
				patch_control_points_count                                    // uint32_t                                   patchControlPoints
			};
		}



		void SpecifyPipelineRasterizationState(bool                                     depth_clamp_enable,
			bool                                     rasterizer_discard_enable,
			VkPolygonMode                            polygon_mode,
			VkCullModeFlags                          culling_mode,
			VkFrontFace                              front_face,
			bool                                     depth_bias_enable,
			float                                    depth_bias_constant_factor,
			float                                    depth_bias_clamp,
			float                                    depth_bias_slope_factor,
			float                                    line_width,
			VkPipelineRasterizationStateCreateInfo & rasterization_state_create_info) {
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

		void SpecifyPipelineMultisampleState(VkSampleCountFlagBits                  sample_count,
			bool                                   per_sample_shading_enable,
			float                                  min_sample_shading,
			VkSampleMask const                   * sample_masks,
			bool                                   alpha_to_coverage_enable,
			bool                                   alpha_to_one_enable,
			VkPipelineMultisampleStateCreateInfo & multisample_state_create_info) {
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

		void SpecifyPipelineDepthAndStencilState(bool                                    depth_test_enable,
			bool                                    depth_write_enable,
			VkCompareOp                             depth_compare_op,
			bool                                    depth_bounds_test_enable,
			float                                   min_depth_bounds,
			float                                   max_depth_bounds,
			bool                                    stencil_test_enable,
			VkStencilOpState                        front_stencil_test_parameters,
			VkStencilOpState                        back_stencil_test_parameters,
			VkPipelineDepthStencilStateCreateInfo & depth_and_stencil_state_create_info) {
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

		void SpecifyPipelineBlendState(bool                                                     logic_op_enable,
			VkLogicOp                                                logic_op,
			std::vector<VkPipelineColorBlendAttachmentState> const & attachment_blend_states,
			std::array<float, 4> const                             & blend_constants,
			VkPipelineColorBlendStateCreateInfo                    & blend_state_create_info) {
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


		void SpecifyPipelineDynamicStates(std::vector<VkDynamicState> const & dynamic_states,
			VkPipelineDynamicStateCreateInfo  & dynamic_state_creat_info) {
			dynamic_state_creat_info = {
				VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,     // VkStructureType                      sType
				nullptr,                                                  // const void                         * pNext
				0,                                                        // VkPipelineDynamicStateCreateFlags    flags
				static_cast<uint32_t>(dynamic_states.size()),             // uint32_t                             dynamicStateCount
				dynamic_states.data()                                     // const VkDynamicState               * pDynamicStates
			};
		}

		bool CreatePipelineLayout(VkDevice                                   logical_device,
			std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
			std::vector<VkPushConstantRange> const   & push_constant_ranges,
			VkPipelineLayout                         & pipeline_layout) {
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

		void SpecifyGraphicsPipelineCreationParameters(VkPipelineCreateFlags                                additional_options,
			std::vector<VkPipelineShaderStageCreateInfo> const & shader_stage_create_infos,
			VkPipelineVertexInputStateCreateInfo const         & vertex_input_state_create_info,
			VkPipelineInputAssemblyStateCreateInfo const       & input_assembly_state_create_info,
			VkPipelineTessellationStateCreateInfo const        * tessellation_state_create_info,
			VkPipelineViewportStateCreateInfo const            * viewport_state_create_info,
			VkPipelineRasterizationStateCreateInfo const       & rasterization_state_create_info,
			VkPipelineMultisampleStateCreateInfo const         * multisample_state_create_info,
			VkPipelineDepthStencilStateCreateInfo const        * depth_and_stencil_state_create_info,
			VkPipelineColorBlendStateCreateInfo const          * blend_state_create_info,
			VkPipelineDynamicStateCreateInfo const             * dynamic_state_creat_info,
			VkPipelineLayout                                     pipeline_layout,
			VkRenderPass                                         render_pass,
			uint32_t                                             subpass,
			VkPipeline                                           base_pipeline_handle,
			int32_t                                              base_pipeline_index,
			VkGraphicsPipelineCreateInfo                       & graphics_pipeline_create_info) {
			graphics_pipeline_create_info = {
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                sType
				nullptr,                                                    // const void                                   * pNext
				additional_options,                                         // VkPipelineCreateFlags                          flags
				static_cast<uint32_t>(shader_stage_create_infos.size()),    // uint32_t                                       stageCount
				shader_stage_create_infos.data(),                           // const VkPipelineShaderStageCreateInfo        * pStages
				&vertex_input_state_create_info,                            // const VkPipelineVertexInputStateCreateInfo   * pVertexInputState
				&input_assembly_state_create_info,                          // const VkPipelineInputAssemblyStateCreateInfo * pInputAssemblyState
				tessellation_state_create_info,                             // const VkPipelineTessellationStateCreateInfo  * pTessellationState
				viewport_state_create_info,                                 // const VkPipelineViewportStateCreateInfo      * pViewportState
				&rasterization_state_create_info,                           // const VkPipelineRasterizationStateCreateInfo * pRasterizationState
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

		bool CreatePipelineCacheObject(VkDevice                           logical_device,
			std::vector<unsigned char> const & cache_data,
			VkPipelineCache                  & pipeline_cache) {
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

		bool RetrieveDataFromPipelineCache(VkDevice                     logical_device,
			VkPipelineCache              pipeline_cache,
			std::vector<unsigned char> & pipeline_cache_data) {
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

		bool MergeMultiplePipelineCacheObjects(VkDevice                             logical_device,
			VkPipelineCache                      target_pipeline_cache,
			std::vector<VkPipelineCache> const & source_pipeline_caches) {
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

		bool CreateGraphicsPipelines(VkDevice                                             logical_device,
			std::vector<VkGraphicsPipelineCreateInfo> const    & graphics_pipeline_create_infos,
			VkPipelineCache                                      pipeline_cache,
			std::vector<VkPipeline>                            & graphics_pipelines) {
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

		bool CreateComputePipeline(VkDevice                                logical_device,
			VkPipelineCreateFlags                   additional_options,
			VkPipelineShaderStageCreateInfo const & compute_shader_stage,
			VkPipelineLayout                        pipeline_layout,
			VkPipeline                              base_pipeline_handle,
			VkPipelineCache                         pipeline_cache,
			VkPipeline                            & compute_pipeline) {
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

		void BindPipelineObject(VkCommandBuffer     command_buffer,
			VkPipelineBindPoint pipeline_type,
			VkPipeline          pipeline) {
			vkCmdBindPipeline(command_buffer, pipeline_type, pipeline);
		}

		bool CreatePipelineLayoutWithCombinedImageSamplerBufferAndPushConstantRanges(VkDevice                                 logical_device,
			std::vector<VkPushConstantRange> const & push_constant_ranges,
			VkDescriptorSetLayout                  & descriptor_set_layout,
			VkPipelineLayout                       & pipeline_layout) {
			std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings = {
				{
					0,                                  // uint32_t               binding
					VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,   // VkDescriptorType       descriptorType
					1,                                  // uint32_t               descriptorCount
					VK_SHADER_STAGE_FRAGMENT_BIT,       // VkShaderStageFlags     stageFlags
					nullptr                             // const VkSampler      * pImmutableSamplers
				},
				{
					1,                                  // uint32_t               binding
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // VkDescriptorType       descriptorType
					1,                                  // uint32_t               descriptorCount
					VK_SHADER_STAGE_VERTEX_BIT,         // VkShaderStageFlags     stageFlags
					nullptr                             // const VkSampler      * pImmutableSamplers
				}
			};

			if (!Descriptor::CreateDescriptorSetLayout(logical_device, descriptor_set_layout_bindings, descriptor_set_layout)) {
				return false;
			}

			if (!CreatePipelineLayout(logical_device, { descriptor_set_layout }, push_constant_ranges, pipeline_layout)) {
				return false;
			}
			return true;
		}


		bool CreateGraphicsPipelineWithVertexAndFragmentShadersDepthTestEnabledAndWithDynamicViewportAndScissorTests(VkDevice                                                 logical_device,
			VkPipelineCreateFlags                                    additional_options,
			std::string const                                      & vertex_shader_filename,
			std::string const                                      & fragment_shader_filename,
			std::vector<VkVertexInputBindingDescription> const     & vertex_input_binding_descriptions,
			std::vector<VkVertexInputAttributeDescription> const   & vertex_attribute_descriptions,
			VkPrimitiveTopology                                      primitive_topology,
			bool                                                     primitive_restart_enable,
			VkPolygonMode                                            polygon_mode,
			VkCullModeFlags                                          culling_mode,
			VkFrontFace                                              front_face,
			bool                                                     logic_op_enable,
			VkLogicOp                                                logic_op,
			std::vector<VkPipelineColorBlendAttachmentState> const & attachment_blend_states,
			std::array<float, 4> const                             & blend_constants,
			VkPipelineLayout                                         pipeline_layout,
			VkRenderPass                                             render_pass,
			uint32_t                                                 subpass,
			VkPipeline                                               base_pipeline_handle,
			VkPipelineCache                                          pipeline_cache,
			std::vector<VkPipeline>                                & graphics_pipeline) {
			std::vector<unsigned char> vertex_shader_spirv;
			if (!GetBinaryFileContents(vertex_shader_filename, vertex_shader_spirv)) {
				return false;
			}

			VkDestroyer(VkShaderModule) vertex_shader_module;
			InitVkDestroyer(logical_device, vertex_shader_module);
			if (!Shader::CreateShaderModule(logical_device, vertex_shader_spirv, *vertex_shader_module)) {
				return false;
			}

			std::vector<unsigned char> fragment_shader_spirv;
			if (!GetBinaryFileContents(fragment_shader_filename, fragment_shader_spirv)) {
				return false;
			}
			VkDestroyer(VkShaderModule) fragment_shader_module;
			InitVkDestroyer(logical_device, fragment_shader_module);
			if (!Shader::CreateShaderModule(logical_device, fragment_shader_spirv, *fragment_shader_module)) {
				return false;
			}

			std::vector<Shader::ShaderStageParameters> shader_stage_params = {
				{
					VK_SHADER_STAGE_VERTEX_BIT,   // VkShaderStageFlagBits        ShaderStage
					*vertex_shader_module,        // VkShaderModule               ShaderModule
					"main",                       // char const                 * EntryPointName
					nullptr                       // VkSpecializationInfo const * SpecializationInfo
				},
				{
					VK_SHADER_STAGE_FRAGMENT_BIT, // VkShaderStageFlagBits        ShaderStage
					*fragment_shader_module,      // VkShaderModule               ShaderModule
					"main",                       // char const                 * EntryPointName
					nullptr                       // VkSpecializationInfo const * SpecializationInfo
				}
			};

			std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
			SpecifyPipelineShaderStages(shader_stage_params, shader_stage_create_infos);

			VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
			SpecifyPipelineVertexInputState(vertex_input_binding_descriptions, vertex_attribute_descriptions, vertex_input_state_create_info);

			VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info;
			SpecifyPipelineInputAssemblyState(primitive_topology, primitive_restart_enable, input_assembly_state_create_info);

			Viewport::ViewportInfo viewport_infos = {
				{                     // std::vector<VkViewport>   Viewports
					{
						0.0f,               // float          x
						0.0f,               // float          y
						500.0f,             // float          width
						500.0f,             // float          height
						0.0f,               // float          minDepth
						1.0f                // float          maxDepth
					}
				},
				{                     // std::vector<VkRect2D>     Scissors
					{
						{                   // VkOffset2D     offset
							0,                  // int32_t        x
							0                   // int32_t        y
						},
						{                   // VkExtent2D     extent
							500,                // uint32_t       width
							500                 // uint32_t       height
						}
					}
				}
			};
			VkPipelineViewportStateCreateInfo viewport_state_create_info;
			SpecifyPipelineViewportAndScissorTestState(viewport_infos, viewport_state_create_info);

			VkPipelineRasterizationStateCreateInfo rasterization_state_create_info;
			SpecifyPipelineRasterizationState(false, false, polygon_mode, culling_mode, front_face, false, 0.0f, 1.0f, 0.0f, 1.0f, rasterization_state_create_info);

			VkPipelineMultisampleStateCreateInfo multisample_state_create_info;
			SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, nullptr, false, false, multisample_state_create_info);

			VkStencilOpState stencil_test_parameters = {
				VK_STENCIL_OP_KEEP,   // VkStencilOp    failOp
				VK_STENCIL_OP_KEEP,   // VkStencilOp    passOp
				VK_STENCIL_OP_KEEP,   // VkStencilOp    depthFailOp
				VK_COMPARE_OP_ALWAYS, // VkCompareOp    compareOp
				0,                    // uint32_t       compareMask
				0,                    // uint32_t       writeMask
				0                     // uint32_t       reference
			};
			VkPipelineDepthStencilStateCreateInfo depth_and_stencil_state_create_info;
			SpecifyPipelineDepthAndStencilState(true, true, VK_COMPARE_OP_LESS_OR_EQUAL, false, 0.0f, 1.0f, false, stencil_test_parameters, stencil_test_parameters, depth_and_stencil_state_create_info);

			VkPipelineColorBlendStateCreateInfo blend_state_create_info;
			SpecifyPipelineBlendState(logic_op_enable, logic_op, attachment_blend_states, blend_constants, blend_state_create_info);

			std::vector<VkDynamicState> dynamic_states = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamic_state_create_info;
			SpecifyPipelineDynamicStates(dynamic_states, dynamic_state_create_info);

			VkGraphicsPipelineCreateInfo graphics_pipeline_create_info;
			SpecifyGraphicsPipelineCreationParameters(additional_options, shader_stage_create_infos, vertex_input_state_create_info, input_assembly_state_create_info, nullptr, &viewport_state_create_info,
				rasterization_state_create_info, &multisample_state_create_info, &depth_and_stencil_state_create_info, &blend_state_create_info, &dynamic_state_create_info, pipeline_layout, render_pass,
				subpass, base_pipeline_handle, -1, graphics_pipeline_create_info);

			if (!CreateGraphicsPipelines(logical_device, { graphics_pipeline_create_info }, pipeline_cache, graphics_pipeline)) {
				return false;
			}
			return true;
		}

		bool CreateMultipleGraphicsPipelinesOnMultipleThreads(VkDevice                                                       logical_device,
			std::string const                                            & pipeline_cache_filename,
			std::vector<std::vector<VkGraphicsPipelineCreateInfo>> const & graphics_pipelines_create_infos,
			std::vector<std::vector<VkPipeline>>                         & graphics_pipelines) {
			// Load cache from file (if available)
			std::vector<unsigned char> cache_data;
			GetBinaryFileContents(pipeline_cache_filename, cache_data);

			// Create cache for each thread, initialize its contents with data loaded from file
			std::vector<VkDestroyer(VkPipelineCache)> pipeline_caches(graphics_pipelines_create_infos.size());
			for (size_t i = 0; i < graphics_pipelines_create_infos.size(); ++i) {
				pipeline_caches[i] = VkDestroyer(VkPipelineCache)();
				InitVkDestroyer(logical_device, pipeline_caches[i]);
				if (!CreatePipelineCacheObject(logical_device, cache_data, *pipeline_caches[i])) {
					return false;
				}
			}

			// Create multiple threads, where each thread creates multiple pipelines using its own cache
			std::vector<std::thread> threads(graphics_pipelines_create_infos.size());
			for (size_t i = 0; i < graphics_pipelines_create_infos.size(); ++i) {
				graphics_pipelines[i].resize(graphics_pipelines_create_infos[i].size());
				threads[i] = std::thread(CreateGraphicsPipelines, logical_device, std::ref(graphics_pipelines_create_infos[i]), *pipeline_caches[i], std::ref(graphics_pipelines[i]));
			}

			// Wait for all threads to finish
			for (size_t i = 0; i < graphics_pipelines_create_infos.size(); ++i) {
				threads[i].join();
			}

			// Merge all the caches into one, retrieve its contents and store them in the file
			VkPipelineCache target_cache = *pipeline_caches.back();
			std::vector<VkPipelineCache> source_caches(pipeline_caches.size() - 1);
			for (size_t i = 0; i < pipeline_caches.size() - 1; ++i) {
				source_caches[i] = *pipeline_caches[i];
			}

			if (!MergeMultiplePipelineCacheObjects(logical_device, target_cache, source_caches)) {
				return false;
			}

			if (!RetrieveDataFromPipelineCache(logical_device, target_cache, cache_data)) {
				return false;
			}

			//if( !SaveBinaryFile( pipeline_cache_filename, cache_data ) ) {
			//  return false;
			//}

			return true;
		}

		void DispatchComputeWork(VkCommandBuffer command_buffer,
			uint32_t        x_size,
			uint32_t        y_size,
			uint32_t        z_size) {
			vkCmdDispatch(command_buffer, x_size, y_size, z_size);
		}


		void DestroyPipeline(VkDevice     logical_device,
			VkPipeline & pipeline) {
			if (VK_NULL_HANDLE != pipeline) {
				vkDestroyPipeline(logical_device, pipeline, nullptr);
				pipeline = VK_NULL_HANDLE;
			}
		}

		void DestroyPipelineCache(VkDevice          logical_device,
			VkPipelineCache & pipeline_cache) {
			if (VK_NULL_HANDLE != pipeline_cache) {
				vkDestroyPipelineCache(logical_device, pipeline_cache, nullptr);
				pipeline_cache = VK_NULL_HANDLE;
			}
		}

		void DestroyPipelineLayout(VkDevice           logical_device,
			VkPipelineLayout & pipeline_layout) {
			if (VK_NULL_HANDLE != pipeline_layout) {
				vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
				pipeline_layout = VK_NULL_HANDLE;
			}
		}
	}
	
} // namespace LavaCake
