#pragma once
#include "AllHeaders.h"
#include "ShaderModule.h"
#include "VertexBuffer.h"
#include "UniformBuffer.h"
#include "Texture.h"
#include "Constant.h"


namespace LavaCake {
	namespace Framework {

		struct uniform {
			UniformBuffer*					buffer;
			int											binding;
			VkShaderStageFlags			stage;
		};

		struct texture{
			TextureBuffer* t;
			int binding;
			VkShaderStageFlags			stage;
		};

		struct frameBuffer {
			FrameBuffer* f;
			int binding;
			VkShaderStageFlags			stage;
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

		class GraphicPipeline
		{
		public:


			GraphicPipeline(vec3f viewportMin, vec3f viewportMax, vec2f scisorMin, vec2f scisorMax);

			/**
			* setup the vertex shader module for this pipeline
			*/
			void setVextexShader(VertexShaderModule*	module);

			/**
			* setup the tesselation control shader module for this pipeline
			*/
			void setTesselationControlModule(TessellationControlShaderModule*	module);

			/**
			* setup the tesselation evaluation shader module for this pipeline
			*/
			void setTesselationEvaluationModule(TessellationEvaluationShaderModule*	module);

			/**
			* setup the geometry shader module for this pipeline
			*/
			void setGeometryModule(GeometryShaderModule*	module);

			/**
			* setup the fragment shader module for this pipeline
			*/
			void setFragmentModule(FragmentShaderModule*	module);

			std::vector<Shader::ShaderStageParameters> getStageParameter();

			/**
			*Prepare the Compute pipeline for it's usage
			*Warning : If a modification is made to the compute pipeline after this operation,
			*The compute pipeline need to be compiled again before being used
			*/
			void compile(VkRenderPass& renderpass);

			/**
			*add a uniform Buffer to the pipeline and scpecify it's binding and shader stage
			*/
			void addUniformBuffer(UniformBuffer * b, VkShaderStageFlags stage, int binding = 0);

			/**
			*add a Texture Buffer to the pipeline and scpecify it's binding and shader stage
			*/
			void addTextureBuffer(TextureBuffer * t, VkShaderStageFlags stage, int binding = 0);

			/**
			*add a FrameBuffer Buffer to the pipeline and scpecify it's binding and shader stage
			*/
			void addFrameBuffer(FrameBuffer * f, VkShaderStageFlags stage, int binding = 0);

			/**
			*add a StorageImage to the pipeline and scpecify it's binding and shader stage
			*/
			void addStorageImage(StorageImage * s, VkShaderStageFlags stage, int binding = 0);

			/**
			*add an attachment to the pipeline and scpecify it's binding and shader stage
			*/
			void addAttachment(Attachment * a, VkShaderStageFlags stage, int binding = 0);

			/**
			*set the vertex buffer to be used for this pipeline
			*/
			void setVeritices(VertexBuffer* buffer);

			/**
			*draw call of the pipeline
			*/
			void draw(const VkCommandBuffer buffer);

			/**
			set the cull mode for the pipeline, if not set the pipeline cull the back faces
			*/
			void SetCullMode(VkCullModeFlagBits cullMode);

			/**
			add a push constant and specify it's shader stage
			*/
			void addPushContant(PushConstant* constant, VkShaderStageFlags flag);

			/**
			specify in which subpass this graphics pipeline will be used 
			*/
			void setSubpassNumber(uint32_t number);

		private:

			void generateDescriptorLayout();
			

			VkDestroyer(VkPipeline)																m_pipeline;
			VkDestroyer(VkPipelineLayout)													m_pipelineLayout;

			VertexShaderModule*																		m_vertexModule = nullptr;
			TessellationControlShaderModule*											m_tesselationControlModule = nullptr;
			TessellationEvaluationShaderModule*										m_tesselationEvaluationModule = nullptr;
			GeometryShaderModule*																	m_geometryModule = nullptr;
			FragmentShaderModule*																	m_fragmentModule = nullptr;

			std::vector<uniform>																	m_uniforms;
			std::vector<texture>																	m_textures;
			std::vector<frameBuffer>															m_frameBuffer;
			std::vector<attachment>																m_attachments;
			std::vector<storageImage>															m_storageImage;

			VkDestroyer(VkDescriptorSetLayout)										m_descriptorSetLayout;
			VkDestroyer(VkDescriptorPool)													m_descriptorPool;
			std::vector<VkDescriptorSet>													m_descriptorSets;

			std::vector<Buffer::BufferDescriptorInfo>							m_bufferDescriptorUpdate;
			std::vector<Image::ImageDescriptorInfo>								m_imageDescriptorUpdate;
			std::vector<constant>																	m_constants;
			std::vector<VkDescriptorPoolSize>											m_descriptorPoolSize;
			std::vector<VkDescriptorSetLayoutBinding>							m_descriptorSetLayoutBinding;
			uint32_t																							m_descriptorCount;

			VkPipelineRasterizationStateCreateInfo								m_rasterizationStateCreateInfo;
			VkPipelineMultisampleStateCreateInfo									m_multisampleStateCreateInfo;
			VkPipelineDepthStencilStateCreateInfo									m_depthStencilStateCreateInfo;
			std::vector<VkPipelineColorBlendAttachmentState>			m_attachmentBlendStates;
			VkPipelineColorBlendStateCreateInfo										m_blendStateCreateInfo;
			std::vector<VkDynamicState>														m_dynamicStates;
			VkPipelineDynamicStateCreateInfo											m_dynamicStateCreateInfo;
			std::vector<VkPipelineShaderStageCreateInfo>					m_shaderStageCreateInfos;
			VkGraphicsPipelineCreateInfo													m_pipelineCreateInfo;


			VkPipelineViewportStateCreateInfo											m_viewportInfo;
			Viewport::ViewportInfo																m_viewportscissor;
			VkPipelineVertexInputStateCreateInfo									m_vertexInfo;
			VertexBuffer*																					m_vertexBuffer;
			VkPipelineInputAssemblyStateCreateInfo								m_inputInfo;

			uint32_t																							m_subpassNumber;

			VkCullModeFlagBits																		m_CullMode = VK_CULL_MODE_BACK_BIT;

			
			
		};
	}
}