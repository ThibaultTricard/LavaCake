#pragma once

#include "lib/imgui/imgui.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "RenderPass.h"
#include "Window.h"

namespace LavaCake {
	namespace Framework {
    static LavaCake::Geometry::vertexFormat imguiformat = LavaCake::Geometry::vertexFormat({ LavaCake::Geometry::POS2,LavaCake::Geometry::UV,LavaCake::Geometry::COL4 });
    
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

    
    void initGui(Window* window, Queue* queue, CommandBuffer* cmdBuff);
    
    /**
     \brief Initialise ImGui and create it's graphic pipeline
     \param queue : a pointer to the queue that will be used to copy data to the Buffer
     \param cmdBuff : the command buffer used for this operation, must not be in a recording state
     \param window a pointer to the current Window
     */
    void initGui(Queue* queue, CommandBuffer& cmdBuff,  Window* window ){
      initGui(window, queue, &cmdBuff);
    }

    /**
     \brief Prepare the gui for the current frame
     \param queue : a pointer to the queue that will be used to copy data to the Buffer
     \param cmdBuff : the command buffer used for this operation, must not be in a recording state
     */
    void prepareGui(Queue* queue, CommandBuffer& cmdBuff);


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
      Framework::VertexBuffer* m_vertexBuffer = nullptr;
      LavaCake::Geometry::Mesh_t* m_mesh = nullptr;


      
    };

    static GLFWmousebuttonfun   s_PrevUserCallbackMousebutton = NULL;
    static GLFWscrollfun        s_PrevUserCallbackScroll = NULL;
    static GLFWkeyfun           s_PrevUserCallbackKey = NULL;
    static GLFWcharfun          s_PrevUserCallbackChar = NULL;
    static GLFWcursorposfun     s_PrevUserCallbackMouseMotion = NULL;
    void prepareInput(GLFWwindow* window); 
	}
}
