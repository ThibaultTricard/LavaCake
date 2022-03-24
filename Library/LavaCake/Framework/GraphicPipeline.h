#pragma once
#include "AllHeaders.h"
#include "Pipeline.h"


namespace LavaCake {
  namespace Framework {

    struct constantRange {
      PushConstant* constant = nullptr;
      VkPushConstantRange range;
    };

    struct vertexBufferConstant {
      VertexBuffer* buffer = nullptr;
      std::vector<constantRange> constant_ranges;
    };

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
      void setVertexModule(const VertexShaderModule& module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a tessellation control shader module
      */
      void setTesselationControlModule(const TessellationControlShaderModule& module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a tessellation evaluation shader module
      */
      void setTesselationEvaluationModule(const TessellationEvaluationShaderModule& module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a geomety shader module
      */
      void setGeometryModule(const GeometryShaderModule& module);

      /**
       \brief Set the vertex shader module for the pipeline
       \param module a pointer to a fragment shader module
      */
      void setFragmentModule(const FragmentShaderModule& module);

      /**
       \brief Set the mesh shader module for the pipeline
       \param module a pointer to a mesh shader module
      */
      void setMeshModule(const MeshShaderModule& module);

      /**
       \brief Set the task shader module for the pipeline
       \param module a pointer to a mesh shader module
      */
      void setTaskModule(const TaskShaderModule& module);

      std::vector<shaderStageParameters> getStageParameter();


      void compile(VkRenderPass& renderpass, uint16_t nbColorAttachments);


      /*
      * \brief set the Vertices info that describe what kind of vertex buffer will be used by the pipeline
      * \param bindingDescriptions describe the binding of the vertex buffer
      * \param topology describe what kind of toplogy the vertex buffer will use
      */
      void setVerticesInfo(const std::vector< VkVertexInputBindingDescription>& bindingDescriptions,
        const std::vector< VkVertexInputAttributeDescription>& attributeDescriptions,
        VkPrimitiveTopology topology);

      /**
       \brief Set the vertex buffer that will be used by the pipeline
       \param vertexBuffer the vertex buffer
      */
      void setVertices(const std::vector<VertexBuffer*>& vertexBuffer);


      /**
       \brief Set the vertex buffer that will be used by the pipeline
       \param vertexBuffer the vertex buffer
      */
      void setVertices(const std::vector<vertexBufferConstant>& vertexBuffer);

      /**
       \brief Register a the draw call of the pipeline into a command buffer
       \param cmdBuff the command buffer
      */
      void draw(CommandBuffer& cmdBuff);

      /**
       \brief Set the cull mode for the pipeline, if not set the pipeline cull the back faces
      */
      void setCullMode(VkCullModeFlagBits cullMode);

      /**
       \brief Register a push constant to the pand specify it's shader stage,
       only one push constant can be used per shader stage
       \param constat a pointer to the push constant
       \param shadeStage a flag representing the shader in witch the constant will be pushed
      */
      //void addPushContant(PushConstant* constant, VkShaderStageFlags shadeStage);

      void setPushContantInfo(const std::vector<VkPushConstantRange>& constantDescriptions);


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
      void setLineWidth(float width) {
        m_lineWidth = width;
      }

      /**
       \brief set the polygon mode used for the pipeline if not set the polgon mode will be VK_POLYGON_MODE_FILL
       \param mode the polygon mode
       */
      void setPolygonMode(VkPolygonMode mode) {
        m_polygonMode = mode;
      }

      /**
       \brief set the number of task launch by this pipeline if not set the number of task be 0.
       this function can be ignored if the pipeline does not use task shader
       \param count the number of task
      */
      void setTaskCount(uint32_t count) {
        m_taskCount = count;
      }

      ~GraphicPipeline() override = default;

    private:

      enum pipelineType {
        Undefined, Graphic, MeshTask
      };


      shaderStageParameters                                 m_vertexModule{};
      shaderStageParameters                                 m_tesselationControlModule{};
      shaderStageParameters                                 m_tesselationEvaluationModule{};
      shaderStageParameters                                 m_geometryModule{};
      shaderStageParameters                                 m_fragmentModule{};
      shaderStageParameters						                      m_meshModule{};
      shaderStageParameters																	m_taskModule{};

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



      std::vector<VkViewport>																m_viewports;
      std::vector<VkRect2D>																	m_scissors;
      VkPipelineVertexInputStateCreateInfo									m_vertexInfo;

      std::vector<vertexBufferConstant>											m_vertexBuffers;
      std::vector<VkPushConstantRange>											m_constantInfos;
      VkPipelineInputAssemblyStateCreateInfo								m_inputInfo;

      uint32_t																							m_subpassNumber;

      VkCullModeFlagBits																		m_cullMode = VK_CULL_MODE_BACK_BIT;
      VkPolygonMode																					m_polygonMode = VK_POLYGON_MODE_FILL;

      VkBool32																							m_alphablending = VK_FALSE;
      float                                                 m_lineWidth = 1.0f;

      uint32_t																							m_taskCount = 0;

      bool																									m_compiled = false;
      pipelineType																					m_type = Undefined;



    };
  }
}
