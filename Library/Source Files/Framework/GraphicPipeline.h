#pragma once
#include "AllHeaders.h"
#include "Pipeline.h"


namespace LavaCake {
	namespace Framework {

		class GraphicPipeline : public Pipeline
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

			std::vector<ShaderStageParameters> getStageParameter();

			/**
			*Prepare the Compute pipeline for it's usage
			*Warning : If a modification is made to the compute pipeline after this operation,
			*The compute pipeline need to be compiled again before being used
			*/
			void compile(VkRenderPass& renderpass);


			void reloadShaders();

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


			void setAlphaBlending(bool v) {
				m_alphablending = v ? VK_TRUE : VK_FALSE;
			};
			
			~GraphicPipeline() {

			}

		private:

			void recompile();

			VertexShaderModule*																		m_vertexModule = nullptr;
			TessellationControlShaderModule*											m_tesselationControlModule = nullptr;
			TessellationEvaluationShaderModule*										m_tesselationEvaluationModule = nullptr;
			GeometryShaderModule*																	m_geometryModule = nullptr;
			FragmentShaderModule*																	m_fragmentModule = nullptr;



			std::vector<constant>																	m_constants;


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


			VkBool32																							m_alphablending = VK_FALSE;

			bool																									m_compiled = false;
		};
	}
}