#pragma once

#include "AllHeaders.h"
#include "ShaderModule.h"
#include "VertexBuffer.h"
#include "UniformBuffer.h"
#include "Texture.h"
#include "Constant.h"
#include "GraphicPipeline.h"

namespace LavaCake {
	namespace Framework {

		class ComputePipeline {

		public : 

			ComputePipeline(){}

			void addUniformBuffer(UniformBuffer * b, int binding = 0) {
				m_uniforms.push_back({ b,binding,VK_SHADER_STAGE_COMPUTE_BIT });
			}

			void addTextureBuffer(TextureBuffer * t, int binding = 0) {
				m_textures.push_back({ t,binding,VK_SHADER_STAGE_COMPUTE_BIT });
			}

			void addStorageImage(StorageImage * s, int binding = 0) {
				m_storageImage.push_back({ s,binding,VK_SHADER_STAGE_COMPUTE_BIT });
			}

			void compile() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				InitVkDestroyer(logical, m_pipelineLayout);
				if (!Pipeline::CreatePipelineLayout(logical, { *m_descriptorSetLayout }, {}, *m_pipelineLayout)) {
					ErrorCheck::setError("Can't create compute pipeline layout");
				}

				std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
				Pipeline::SpecifyPipelineShaderStages({ m_computeModule->getStageParameter() }, shader_stage_create_infos);

				InitVkDestroyer(logical, m_pipeline);
				if (!Pipeline::CreateComputePipeline(logical, 0, shader_stage_create_infos[0], *m_pipelineLayout, VK_NULL_HANDLE, VK_NULL_HANDLE, *m_pipeline)) {
					ErrorCheck::setError("Can't create compute pipeline");
				}
			}

			void setComputeShader(ComputeShaderModule*	module) {
				m_computeModule = module;
			}

			void compute(const VkCommandBuffer buffer, uint32_t dimX, uint32_t dimY, uint32_t dimZ) {
				Descriptor::BindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, *m_pipelineLayout, 0, m_descriptorSets, {});

				Pipeline::BindPipelineObject(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, *m_pipeline);

				Pipeline::DispatchComputeWork(buffer, dimX, dimY, dimZ);
			}

		private :

			void generateDescriptorLayout() {

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


				m_descriptorPoolSize = {};

				if (m_uniforms.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,							// VkDescriptorType     type
							uint32_t(m_uniforms.size())										// uint32_t             descriptorCount
						});
				}
				if (m_textures.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,			// VkDescriptorType     type
							uint32_t(m_textures.size())										// uint32_t             descriptorCount
						});
				}
				if (m_attachments.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,						// VkDescriptorType     type
							uint32_t(m_attachments.size())								// uint32_t             descriptorCount
						});
				}
				if (m_storageImage.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,								// VkDescriptorType     type
							uint32_t(m_storageImage.size())								// uint32_t             descriptorCount
						});
				}

				uint32_t max_sets_count = m_uniforms.size() + m_textures.size() + m_attachments.size();
				InitVkDestroyer(logical, m_descriptorPool);
				if (!LavaCake::Descriptor::CreateDescriptorPool(logical, false, max_sets_count, m_descriptorPoolSize, *m_descriptorPool)) {
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

			VkDestroyer(VkPipeline)																m_pipeline;
			VkDestroyer(VkPipelineLayout)													m_pipelineLayout;
			
			std::vector<uniform>																	m_uniforms;
			std::vector<texture>																	m_textures;
			std::vector<attachment>																m_attachments;
			std::vector<storageImage>															m_storageImage;

			std::vector<Buffer::BufferDescriptorInfo>							m_bufferDescriptorUpdate;
			std::vector<Image::ImageDescriptorInfo>								m_imageDescriptorUpdate;

			VkDestroyer(VkDescriptorSetLayout)										m_descriptorSetLayout;
			VkDestroyer(VkDescriptorPool)													m_descriptorPool;
			std::vector<VkDescriptorSet>													m_descriptorSets;
			std::vector<VkDescriptorPoolSize>											m_descriptorPoolSize;
			std::vector<VkDescriptorSetLayoutBinding>							m_descriptorSetLayoutBinding;

			ComputeShaderModule*																	m_computeModule;


		};
	}
}