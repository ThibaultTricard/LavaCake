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
			Buffer* t;
			int binding;
			VkShaderStageFlags			stage;
		};
		struct buffer {
			Buffer* t;
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


			virtual void addTexelBuffer(Buffer* a, VkShaderStageFlags stage, int binding = 0) {
				m_texelBuffers.push_back({ a,binding,stage });
			};

			virtual void addBuffer(Buffer* a, VkShaderStageFlags stage, int binding = 0) {
				m_buffers.push_back({ a,binding,stage });
			};

			std::vector<attachment>& getAttachments() {
				return m_attachments;
			};

			~Pipeline() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				if (VK_NULL_HANDLE != *m_pipeline) {
					vkDestroyPipeline(logical, *m_pipeline, nullptr);
					*m_pipeline = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_pipelineLayout) {
					vkDestroyPipelineLayout(logical, *m_pipelineLayout, nullptr);
					*m_pipelineLayout = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_descriptorPool) {
					vkDestroyDescriptorPool(logical, *m_descriptorPool, nullptr);
					*m_descriptorPool = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_descriptorSetLayout) {
					vkDestroyDescriptorSetLayout(logical, *m_descriptorSetLayout, nullptr);
					*m_descriptorSetLayout = VK_NULL_HANDLE;
				}
			};

		protected :
			virtual void generateDescriptorLayout();

			void SpecifyPipelineShaderStages(std::vector<Framework::ShaderStageParameters> const& shader_stage_params,
				std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_create_infos);


			bool CreatePipelineLayout(VkDevice                                   logical_device,
				std::vector<VkDescriptorSetLayout> const& descriptor_set_layouts,
				std::vector<VkPushConstantRange> const& push_constant_ranges,
				VkPipelineLayout& pipeline_layout);

			bool CreateGraphicsPipelines(VkDevice                                             logical_device,
				std::vector<VkGraphicsPipelineCreateInfo> const& graphics_pipeline_create_infos,
				VkPipelineCache                                      pipeline_cache,
				std::vector<VkPipeline>& graphics_pipelines);




			VkDestroyer(VkPipeline)																					m_pipeline;
			VkDestroyer(VkPipelineLayout)																		m_pipelineLayout;

			VkDestroyer(VkDescriptorSetLayout)															m_descriptorSetLayout;
			VkDestroyer(VkDescriptorPool)																		m_descriptorPool;
			std::vector<VkDescriptorSet>																		m_descriptorSets;
			std::vector<VkDescriptorPoolSize>																m_descriptorPoolSize;
			std::vector<VkDescriptorSetLayoutBinding>												m_descriptorSetLayoutBinding;
			uint32_t																												m_descriptorCount;

			std::vector<Core::BufferDescriptorInfo>													m_bufferDescriptorUpdate;
			std::vector<Core::ImageDescriptorInfo>													m_imageDescriptorUpdate;
			std::vector<Core::TexelBufferDescriptorInfo>										m_texelBufferDescriptorUpdate;

			std::vector<uniform>																						m_uniforms;
			std::vector<texture>																						m_textures;
			std::vector<frameBuffer>																				m_frameBuffers;
			std::vector<attachment>																					m_attachments;
			std::vector<storageImage>																				m_storageImages;
			std::vector<texelBuffer>																				m_texelBuffers;
			std::vector<buffer>																							m_buffers;
			std::vector<constant>																						m_constants;
		};
	}
}