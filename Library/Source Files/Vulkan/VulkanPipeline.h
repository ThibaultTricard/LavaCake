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
//
// Chapter: 08 Graphics and Compute Pipelines
// Recipe:  01 Creating a shader module

#ifndef VULKAN_PIPELINE
#define VULKAN_PIPELINE

#include "Common.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptor.h"
#include "VulkanViewPort.h"
#include "VulkanShader.h"
#include "Tools.h"

namespace LavaCake {
	namespace Pipeline {
		void SpecifyPipelineShaderStages(std::vector<Shader::ShaderStageParameters> const     & shader_stage_params,
			std::vector<VkPipelineShaderStageCreateInfo> & shader_stage_create_infos);

		void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription> const   & binding_descriptions,
			std::vector<VkVertexInputAttributeDescription> const & attribute_descriptions,
			VkPipelineVertexInputStateCreateInfo                 & vertex_input_state_create_info);

		void SpecifyPipelineInputAssemblyState(VkPrimitiveTopology                      topology,
			bool                                     primitive_restart_enable,
			VkPipelineInputAssemblyStateCreateInfo & input_assembly_state_create_info);

		void SpecifyPipelineTessellationState(uint32_t                                patch_control_points_count,
			VkPipelineTessellationStateCreateInfo & tessellation_state_create_info);


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
			VkPipelineRasterizationStateCreateInfo & rasterization_state_create_info);

		void SpecifyPipelineMultisampleState(VkSampleCountFlagBits                  sample_count,
			bool                                   per_sample_shading_enable,
			float                                  min_sample_shading,
			VkSampleMask const                   * sample_masks,
			bool                                   alpha_to_coverage_enable,
			bool                                   alpha_to_one_enable,
			VkPipelineMultisampleStateCreateInfo & multisample_state_create_info);

		void SpecifyPipelineDepthAndStencilState(bool                                    depth_test_enable,
			bool                                    depth_write_enable,
			VkCompareOp                             depth_compare_op,
			bool                                    depth_bounds_test_enable,
			float                                   min_depth_bounds,
			float                                   max_depth_bounds,
			bool                                    stencil_test_enable,
			VkStencilOpState                        front_stencil_test_parameters,
			VkStencilOpState                        back_stencil_test_parameters,
			VkPipelineDepthStencilStateCreateInfo & depth_and_stencil_state_create_info);

		void SpecifyPipelineBlendState(bool                                                     logic_op_enable,
			VkLogicOp                                                logic_op,
			std::vector<VkPipelineColorBlendAttachmentState> const & attachment_blend_states,
			std::array<float, 4> const                             & blend_constants,
			VkPipelineColorBlendStateCreateInfo                    & blend_state_create_info);

		void SpecifyPipelineDynamicStates(std::vector<VkDynamicState> const & dynamic_states,
			VkPipelineDynamicStateCreateInfo  & dynamic_state_creat_info);

		bool CreatePipelineLayout(VkDevice                                   logical_device,
			std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
			std::vector<VkPushConstantRange> const   & push_constant_ranges,
			VkPipelineLayout                         & pipeline_layout);

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
			VkGraphicsPipelineCreateInfo                       & graphics_pipeline_create_info);

		bool CreatePipelineCacheObject(VkDevice                           logical_device,
			std::vector<unsigned char> const & cache_data,
			VkPipelineCache                  & pipeline_cache);

		bool RetrieveDataFromPipelineCache(VkDevice                     logical_device,
			VkPipelineCache              pipeline_cache,
			std::vector<unsigned char> & pipeline_cache_data);

		bool RetrieveDataFromPipelineCache(VkDevice                     logical_device,
			VkPipelineCache              pipeline_cache,
			std::vector<unsigned char> & pipeline_cache_data);

		bool MergeMultiplePipelineCacheObjects(VkDevice                             logical_device,
			VkPipelineCache                      target_pipeline_cache,
			std::vector<VkPipelineCache> const & source_pipeline_caches);

		bool CreateGraphicsPipelines(VkDevice                                             logical_device,
			std::vector<VkGraphicsPipelineCreateInfo> const    & graphics_pipeline_create_infos,
			VkPipelineCache                                      pipeline_cache,
			std::vector<VkPipeline>                            & graphics_pipelines);

		bool CreateComputePipeline(VkDevice                                logical_device,
			VkPipelineCreateFlags                   additional_options,
			VkPipelineShaderStageCreateInfo const & compute_shader_stage,
			VkPipelineLayout                        pipeline_layout,
			VkPipeline                              base_pipeline_handle,
			VkPipelineCache                         pipeline_cache,
			VkPipeline                            & compute_pipeline);

		void BindPipelineObject(VkCommandBuffer     command_buffer,
			VkPipelineBindPoint pipeline_type,
			VkPipeline          pipeline);

		bool CreatePipelineLayoutWithCombinedImageSamplerBufferAndPushConstantRanges(VkDevice                                 logical_device,
			std::vector<VkPushConstantRange> const & push_constant_ranges,
			VkDescriptorSetLayout                  & descriptor_set_layout,
			VkPipelineLayout                       & pipeline_layout);

		bool CreateGraphicsPipelineWithVertexAndFragmentShadersDepthTestEnabledAndWithDynamicViewportAndScissorTests(
			VkDevice                                                 logical_device,
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
			std::vector<VkPipeline>                                & graphics_pipeline);

		bool CreateMultipleGraphicsPipelinesOnMultipleThreads(VkDevice                                                       logical_device,
			std::string const                                            & pipeline_cache_filename,
			std::vector<std::vector<VkGraphicsPipelineCreateInfo>> const & graphics_pipelines_create_infos,
			std::vector<std::vector<VkPipeline>>                         & graphics_pipelines);

		void DispatchComputeWork(
			VkCommandBuffer command_buffer,
			uint32_t        x_size,
			uint32_t        y_size,
			uint32_t        z_size);

		void DestroyPipeline(VkDevice     logical_device,
			VkPipeline & pipeline);

		void DestroyPipelineCache(VkDevice          logical_device,
			VkPipelineCache & pipeline_cache);

		void DestroyPipelineLayout(VkDevice           logical_device,
			VkPipelineLayout & pipeline_layout);
	}
} // namespace LavaCake

#endif // CREATING_A_SHADER_MODULE