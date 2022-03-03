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
     */
    ImGuiWrapper() {};

    /**
     \brief Initialise ImGui and create it's graphic pipeline
     \param queue : a pointer to the queue that will be used to copy data to the Buffer
     \param cmdBuff : the command buffer used for this operation, must not be in a recording state
     \param windowSize: the size of the window
     \param frameBufferSize: the size of the frame buffer imgui is used in
     */
    void initGui(Queue* queue, CommandBuffer& cmdBuff, const vec2i& windowSize, const vec2i& frameBufferSize);

    /**
     \brief Prepare the gui for the current frame
     \param queue : a pointer to the queue that will be used to copy data to the Buffer
     \param cmdBuff : the command buffer used for this operation, must not be in a recording state
     */
    void prepareGui(Queue* queue, CommandBuffer& cmdBuff);

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
      GraphicPipeline* getPipeline() {
        return m_pipeline;
      }

      ~ImGuiWrapper() {
        delete m_pipeline;
        delete m_pushConstant;
        delete m_vertexBuffer;
      }

    private : 
      
      GraphicPipeline* m_pipeline = nullptr;
      PushConstant* m_pushConstant = nullptr;
      VertexBuffer* m_vertexBuffer = nullptr;
      Geometry::Mesh_t* m_mesh = nullptr;
      Image* m_fontBuffer = nullptr;
      
    };


#if defined(LAVACAKE_WINDOW_MANAGER_GLFW)
    static GLFWmousebuttonfun   s_PrevUserCallbackMousebutton = NULL;
    static GLFWscrollfun        s_PrevUserCallbackScroll = NULL;
    static GLFWkeyfun           s_PrevUserCallbackKey = NULL;
    static GLFWcharfun          s_PrevUserCallbackChar = NULL;
    static GLFWcursorposfun     s_PrevUserCallbackMouseMotion = NULL; 



    void prepareInputs(GLFWwindow* window);
#endif
	}
}
