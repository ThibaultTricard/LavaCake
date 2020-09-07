#pragma once

#include "ShaderModule.h"
#include "VertexBuffer.h"
#include "UniformBuffer.h"
#include "Texture.h"
#include "Constant.h"

namespace LavaCake {
	namespace Framework {

		struct uniform {
			UniformBuffer* buffer;
			int											binding;
			VkShaderStageFlags			stage;
		};

		struct texture {
			TextureBuffer* t;
			int binding;
			VkShaderStageFlags			stage;
		};

		struct frameBuffer {
			FrameBuffer* f;
			int binding;
			VkShaderStageFlags			stage;
			uint32_t viewIndex;
		};

		struct attachment {
			Attachment* a;
			int binding;
			VkShaderStageFlags			stage;
		};

		struct storageImage {
			StorageImage* s;
			int binding;
			VkShaderStageFlags			stage;
		};

		struct constant {
			PushConstant* constant;
			VkShaderStageFlags			stage;
		};

		struct texelBuffer {
			TexelBuffer* t;
			int binding;
			VkShaderStageFlags			stage;
		};

		class Pipeline {
		public :
			/**
				*add a uniform Buffer to the pipeline and scpecify it's binding and shader stage
				*/
			virtual void addUniformBuffer(UniformBuffer* b, VkShaderStageFlags stage, int binding = 0) {
				m_uniforms.push_back({ b,binding,stage });
			};

			/**
			*add a Texture Buffer to the pipeline and scpecify it's binding and shader stage
			*/
			virtual void addTextureBuffer(TextureBuffer* t, VkShaderStageFlags stage, int binding = 0) {
				m_textures.push_back({ t,binding,stage });
			};

			/**
			*add a FrameBuffer Buffer to the pipeline and scpecify it's binding and shader stage
			*/
			virtual void addFrameBuffer(FrameBuffer* f, VkShaderStageFlags stage, int binding = 0, uint32_t view = 0) {
				m_frameBuffers.push_back({ f, binding, stage, view });
			}; 

			/**
			*add a StorageImage to the pipeline and scpecify it's binding and shader stage
			*/
			virtual void addStorageImage(StorageImage* s, VkShaderStageFlags stage, int binding = 0) {
				m_storageImages.push_back({ s,binding,stage });
			}; 


			/**
			*add an attachment to the pipeline and scpecify it's binding and shader stage
			*/
			virtual void addAttachment(Attachment* a, VkShaderStageFlags stage, int binding = 0) {
				m_attachments.push_back({ a,binding,stage });
			};


			virtual void addTexelBuffer(TexelBuffer* a, VkShaderStageFlags stage, int binding = 0) {
				m_texelBuffers.push_back({ a,binding,stage });
			};

			std::vector<attachment>& getAttachments() {
				return m_attachments;
			};

			~Pipeline() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				DestroyPipeline(logical, *m_pipeline);
				DestroyPipelineLayout(logical, *m_pipelineLayout);
				Descriptor::DestroyDescriptorPool(logical, *m_descriptorPool);
				Descriptor::DestroyDescriptorSetLayout(logical, *m_descriptorSetLayout);
			};

		protected :
			virtual void generateDescriptorLayout();

			void SpecifyPipelineShaderStages(std::vector<Framework::ShaderStageParameters> const& shader_stage_params,
				std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_create_infos);

			void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription> const& binding_descriptions,
				std::vector<VkVertexInputAttributeDescription> const& attribute_descriptions,
				VkPipelineVertexInputStateCreateInfo& vertex_input_state_create_info);

			void SpecifyPipelineInputAssemblyState(VkPrimitiveTopology                      topology,
				bool                                     primitive_restart_enable,
				VkPipelineInputAssemblyStateCreateInfo& input_assembly_state_create_info);

			void SpecifyPipelineTessellationState(uint32_t                                patch_control_points_count,
				VkPipelineTessellationStateCreateInfo& tessellation_state_create_info);

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
				VkPipelineRasterizationStateCreateInfo& rasterization_state_create_info);

			void SpecifyPipelineMultisampleState(VkSampleCountFlagBits                  sample_count,
				bool                                   per_sample_shading_enable,
				float                                  min_sample_shading,
				VkSampleMask const* sample_masks,
				bool                                   alpha_to_coverage_enable,
				bool                                   alpha_to_one_enable,
				VkPipelineMultisampleStateCreateInfo& multisample_state_create_info);

			void SpecifyPipelineDepthAndStencilState(bool                                    depth_test_enable,
				bool                                    depth_write_enable,
				VkCompareOp                             depth_compare_op,
				bool                                    depth_bounds_test_enable,
				float                                   min_depth_bounds,
				float                                   max_depth_bounds,
				bool                                    stencil_test_enable,
				VkStencilOpState                        front_stencil_test_parameters,
				VkStencilOpState                        back_stencil_test_parameters,
				VkPipelineDepthStencilStateCreateInfo& depth_and_stencil_state_create_info);

			void SpecifyPipelineBlendState(bool                                                     logic_op_enable,
				VkLogicOp                                                logic_op,
				std::vector<VkPipelineColorBlendAttachmentState> const& attachment_blend_states,
				std::array<float, 4> const& blend_constants,
				VkPipelineColorBlendStateCreateInfo& blend_state_create_info);

			void SpecifyPipelineDynamicStates(std::vector<VkDynamicState> const& dynamic_states,
				VkPipelineDynamicStateCreateInfo& dynamic_state_creat_info);

			bool CreatePipelineLayout(VkDevice                                   logical_device,
				std::vector<VkDescriptorSetLayout> const& descriptor_set_layouts,
				std::vector<VkPushConstantRange> const& push_constant_ranges,
				VkPipelineLayout& pipeline_layout);

			void SpecifyGraphicsPipelineCreationParameters(VkPipelineCreateFlags                                additional_options,
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
				VkGraphicsPipelineCreateInfo& graphics_pipeline_create_info);

			bool CreatePipelineCacheObject(VkDevice                           logical_device,
				std::vector<unsigned char> const& cache_data,
				VkPipelineCache& pipeline_cache);

			bool RetrieveDataFromPipelineCache(VkDevice                     logical_device,
				VkPipelineCache              pipeline_cache,
				std::vector<unsigned char>& pipeline_cache_data);

			bool MergeMultiplePipelineCacheObjects(VkDevice                             logical_device,
				VkPipelineCache                      target_pipeline_cache,
				std::vector<VkPipelineCache> const& source_pipeline_caches);

			bool CreateGraphicsPipelines(VkDevice                                             logical_device,
				std::vector<VkGraphicsPipelineCreateInfo> const& graphics_pipeline_create_infos,
				VkPipelineCache                                      pipeline_cache,
				std::vector<VkPipeline>& graphics_pipelines);

			bool CreateComputePipeline(VkDevice                                logical_device,
				VkPipelineCreateFlags                   additional_options,
				VkPipelineShaderStageCreateInfo const& compute_shader_stage,
				VkPipelineLayout                        pipeline_layout,
				VkPipeline                              base_pipeline_handle,
				VkPipelineCache                         pipeline_cache,
				VkPipeline& compute_pipeline);

			void BindPipelineObject(VkCommandBuffer     command_buffer,
				VkPipelineBindPoint pipeline_type,
				VkPipeline          pipeline);


			void DispatchComputeWork(
				VkCommandBuffer command_buffer,
				uint32_t        x_size,
				uint32_t        y_size,
				uint32_t        z_size);

			void DestroyPipeline(VkDevice     logical_device,
				VkPipeline& pipeline);

			void DestroyPipelineCache(VkDevice          logical_device,
				VkPipelineCache& pipeline_cache);

			void DestroyPipelineLayout(VkDevice           logical_device,
				VkPipelineLayout& pipeline_layout);


			


			VkDestroyer(VkPipeline)																m_pipeline;
			VkDestroyer(VkPipelineLayout)													m_pipelineLayout;

			VkDestroyer(VkDescriptorSetLayout)										m_descriptorSetLayout;
			VkDestroyer(VkDescriptorPool)													m_descriptorPool;
			std::vector<VkDescriptorSet>													m_descriptorSets;
			std::vector<VkDescriptorPoolSize>											m_descriptorPoolSize;
			std::vector<VkDescriptorSetLayoutBinding>							m_descriptorSetLayoutBinding;
			uint32_t																							m_descriptorCount;

			std::vector<Buffer::BufferDescriptorInfo>							m_bufferDescriptorUpdate;
			std::vector<Image::ImageDescriptorInfo>								m_imageDescriptorUpdate;
			std::vector<Buffer::TexelBufferDescriptorInfo>				m_texelBufferDescriptorUpdate;

			std::vector<uniform>																	m_uniforms;
			std::vector<texture>																	m_textures;
			std::vector<frameBuffer>															m_frameBuffers;
			std::vector<attachment>																m_attachments;
			std::vector<storageImage>															m_storageImages;
			std::vector<texelBuffer>															m_texelBuffers;

		};
	}
}