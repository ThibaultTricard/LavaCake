#pragma once
#include "AllHeaders.h"
#include "Pipeline.h"


namespace LavaCake {
	namespace Framework {

  /**
   Class GraphicPipeline :
   \brief Helps manage a compute pipeline and it's binding
   Inherit the pipeline Pipeline class
   */
		class GraphicPipeline : public Pipeline
		{
		public:

      /**
       \brief Constructor for the gaphic pipelin class
       \param viewportMin the minimum coordinate of view port
       \param viewportMax the maximum coordinate of view port
       \param scisorMin the minimum coordinate on wich the graphic pipeline will draw
       \param scisorMax the maximum coordinate on wich the graphic pipeline will draw
      */
			GraphicPipeline(vec3f viewportMin, vec3f viewportMax, vec2f scissorMin, vec2f scissorMax);

			/**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a vertex shader module
			*/
			void setVertexModule(VertexShaderModule*	module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a tessellation control shader module
      */
			void setTesselationControlModule(TessellationControlShaderModule*	module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a tessellation evaluation shader module
      */
			void setTesselationEvaluationModule(TessellationEvaluationShaderModule*	module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a geomety shader module
      */
			void setGeometryModule(GeometryShaderModule*	module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a fragment shader module
      */
			void setFragmentModule(FragmentShaderModule*	module);

			std::vector<ShaderStageParameters> getStageParameter();

			
			void compile(VkRenderPass& renderpass, uint16_t nbColorAttachments);

      /**
       \brief Reload all the spirv shaders module from source
       */
			void reloadShaders();

			/**
       \brief Set the vertex buffer that will be used by the pipeline
       \param vertexBuffer the vertex buffer
			*/
			void setVertices(VertexBuffer* vertexBuffer);

			/**
       \brief Register a the draw call of the pipeline into a command buffer
       \param cmdBuff the command buffer
			*/
			void draw(CommandBuffer& cmdBuff);

			/**
       \brief Set the cull mode for the pipeline, if not set the pipeline cull the back faces
			*/
			void SetCullMode(VkCullModeFlagBits cullMode);

			/**
       \brief Register a push constant to the pand specify it's shader stage,
       only one push constant can be used per shader stage
       \param constat a pointer to the push constant
       \param shadeStage a flag representing the shader in witch the constant will be pushed
			*/
			void addPushContant(PushConstant* constant, VkShaderStageFlags shadeStage);

	
			void setSubpassNumber(uint32_t number);

      /**
       \brief enable or disable the alpha blending, if not set the alpha blendig is disable
       \param alphaBlending a boolean representing wether or not to enable the alphablending
       */
			void setAlphaBlending(bool alphaBlending) {
				m_alphablending = alphaBlending ? VK_TRUE : VK_FALSE;
			};
      
      /**
       \brief set the line width used to draw line in the pipeline, if not set the line width will be set to 1.0f
       \param width the line width
       */
      void setLineWidth(float width){
        m_lineWidth =width;
      }
			
			~GraphicPipeline() {

			}

		private:

			void recompile();

			VertexShaderModule*																		m_vertexModule = nullptr;
			TessellationControlShaderModule*											m_tesselationControlModule = nullptr;
			TessellationEvaluationShaderModule*										m_tesselationEvaluationModule = nullptr;
			GeometryShaderModule*																	m_geometryModule = nullptr;
			FragmentShaderModule*																	m_fragmentModule = nullptr;



			


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
			LavaCake::Core::ViewportInfo													m_viewportscissor;
			VkPipelineVertexInputStateCreateInfo									m_vertexInfo;
			VertexBuffer*																					m_vertexBuffer;
			VkPipelineInputAssemblyStateCreateInfo								m_inputInfo;

			uint32_t																							m_subpassNumber;

			VkCullModeFlagBits																		m_CullMode = VK_CULL_MODE_BACK_BIT;


			VkBool32																							m_alphablending = VK_FALSE;
      float                                                 m_lineWidth = 1.0f;

			bool																									m_compiled = false;
		};
	}
}
