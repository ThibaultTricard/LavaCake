#pragma once

#include "lib/imgui/imgui.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "RenderPass.h"

namespace LavaCake {
	namespace Framework {
    static Geometry::vertexFormat imguiformat =Geometry::vertexFormat({ Geometry::POS2,Geometry::UV,Geometry::COL4 });
    
  /**
   Class ImGuiWrapper :
   \brief Wrap Imgui into a Lavacake::Pipeline
   */
  class ImGuiWrapper {
    public :
    
    /**
     \brief Default Constructor
     \param queue : a pointer to the queue that will be used to copy data to the Buffer
     \param cmdBuff : the command buffer used for this operation, must not be in a recording state
     \param windowSize: the size of the window
     \param frameBufferSize: the size of the frame buffer imgui is used in
     */
     ImGuiWrapper(const Queue& queue, CommandBuffer& cmdBuff, const vec2i& windowSize, const vec2i& frameBufferSize);


    /**
     \brief Prepare the gui for the current frame
     \param queue : a pointer to the queue that will be used to copy data to the Buffer
     \param cmdBuff : the command buffer used for this operation, must not be in a recording state
     */
    void prepareGui(const Queue& queue, CommandBuffer& cmdBuff);

    /**
     \brief resize the gui
     \param windowSize: the new size of the window
     \param frameBufferSize: the new size of the frame buffer imgui is used in
     */
    void resizeGui(const vec2i& windowSize, const vec2i& frameBufferSize);

    /**
     \brief Return the graphic pipelin for the gui
     \return a pointer to the graphic pipeline
     */
    std::shared_ptr < GraphicPipeline > getPipeline() const{
      return m_pipeline;
    }

    ~ImGuiWrapper() {
    }

    private : 
      
      std::shared_ptr< GraphicPipeline >     m_pipeline;
      PushConstant                           m_pushConstant;
      std::unique_ptr < VertexBuffer >       m_vertexBuffer;
      std::unique_ptr < Geometry::Mesh_t >   m_mesh;
      std::shared_ptr < Image >              m_fontBuffer;

      std::unique_ptr < VertexShaderModule >    m_vertexShader;
      std::unique_ptr < FragmentShaderModule >  m_fragmentShader;
      std::shared_ptr < DescriptorSet >         m_descritporSet;
      
    };


#if defined(LAVACAKE_WINDOW_MANAGER_GLFW)
    static GLFWmousebuttonfun   s_PrevUserCallbackMousebutton = NULL;
    static GLFWscrollfun        s_PrevUserCallbackScroll = NULL;
    static GLFWkeyfun           s_PrevUserCallbackKey = NULL;
    static GLFWcharfun          s_PrevUserCallbackChar = NULL;
    static GLFWcursorposfun     s_PrevUserCallbackMouseMotion = NULL; 


    /**
     \brief bind GLWF inputs to Imgui
     \params window a pointer to the GLFWwindow
     */
    void prepareInputs(GLFWwindow* window);
#endif
	}
}
