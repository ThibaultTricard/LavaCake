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

			/**
			*Constructor of the class
			*/
			ComputePipeline() {};

			/**
			*add a UniformBufffer b to a list of uniform bufffer and specify it's future binding
			*/
			void addUniformBuffer(UniformBuffer * b, int binding = 0);
			
			/**
			*add a TextureBufffer b to a list of Texture bufffer and specify it's future binding 
			*/
			void addTextureBuffer(TextureBuffer * t, int binding = 0);

			/**
			*add a StorageImage s to a list of Storage Image and specify it's future binding 
			*/
			void addStorageImage(StorageImage * s, int binding = 0);

			/**
			*Specify the compute shader module that will be used in the pipeline
			*/
			void setComputeShader(ComputeShaderModule*	module);
			
			/**
			*Prepare the Compute pipeline for it's usage
			*Warning : If a modification is made to the compute pipeline after this operation,
			*The compute pipeline need to be compiled again before being used
			*/
			void compile();
			
			/**
			*dispach the compute shader
			*/
			void compute(const VkCommandBuffer buffer, uint32_t dimX, uint32_t dimY, uint32_t dimZ);


		private :

			void generateDescriptorLayout();

			VkDestroyer(VkPipeline)																m_pipeline;
			VkDestroyer(VkPipelineLayout)													m_pipelineLayout;
			
			std::vector<uniform>																	m_uniforms;
			std::vector<texture>																	m_textures;
			std::vector<storageImage>															m_storageImage;

			std::vector<Buffer::BufferDescriptorInfo>							m_bufferDescriptorUpdate;
			std::vector<Image::ImageDescriptorInfo>								m_imageDescriptorUpdate;

			VkDestroyer(VkDescriptorSetLayout)										m_descriptorSetLayout;
			VkDestroyer(VkDescriptorPool)													m_descriptorPool;
			std::vector<VkDescriptorSet>													m_descriptorSets;
			std::vector<VkDescriptorPoolSize>											m_descriptorPoolSize;
			std::vector<VkDescriptorSetLayoutBinding>							m_descriptorSetLayoutBinding;
			uint32_t																							m_descriptorCount;

			ComputeShaderModule*																	m_computeModule;


		};
	}
}