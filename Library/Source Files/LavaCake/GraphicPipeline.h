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

			void setVextexShader(VertexShaderModule*	module);

			void setTesselationControlModule(TessellationControlShaderModule*	module);

			void setTesselationEvaluationModule(TessellationEvaluationShaderModule*	module);

			void setGeometryModule(GeometryShaderModule*	module);

			void setFragmentModule(FragmentShaderModule*	module);

			std::vector<Shader::ShaderStageParameters> getStageParameter();

			void compile(VkRenderPass& renderpass);

			void addUniformBuffer(UniformBuffer * b, VkShaderStageFlags stage, int binding = 0);

			void addTextureBuffer(TextureBuffer * t, VkShaderStageFlags stage, int binding = 0);

			void addAttachment(Attachment * a, VkShaderStageFlags stage, int binding = 0);

			void setVeritices(VertexBuffer* buffer);

			void draw(const VkCommandBuffer buffer);

			uint32_t getSubpassNumber();

			void SetCullMode(VkCullModeFlagBits cullMode);

			void addPushContant(PushConstant* constant, VkShaderStageFlags flag);

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
			std::vector<attachment>																m_attachments;
			VkDestroyer(VkDescriptorSetLayout)										m_descriptorSetLayout;
			VkDestroyer(VkDescriptorPool)													m_descriptorPool;
			std::vector<VkDescriptorSet>													m_descriptorSets;

			std::vector<Buffer::BufferDescriptorInfo>							m_bufferDescriptorUpdate;
			std::vector<Image::ImageDescriptorInfo>								m_imageDescriptorUpdate;
			std::vector<constant>																	m_constants;
			std::vector<VkDescriptorPoolSize>											m_descriptorPoolSize;
			std::vector<VkDescriptorSetLayoutBinding>							m_descriptorSetLayoutBinding;


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