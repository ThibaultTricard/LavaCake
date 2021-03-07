#pragma once

#include "lib/imgui/imgui.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "RenderPass.h"
#include "Window.h"

namespace LavaCake {
	namespace Framework {
    static LavaCake::Geometry::vertexFormat imguiformat = LavaCake::Geometry::vertexFormat({ LavaCake::Geometry::POS2,LavaCake::Geometry::UV,LavaCake::Geometry::COL4 });
    class ImGuiWrapper {
    public : 
      ImGuiWrapper() {};

      void initGui(Window* win, Queue* queue, CommandBuffer* cmdBuff);

      void prepareGui(Queue* queue, CommandBuffer* cmdBuff);

       

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
    void prepareImput(GLFWwindow* window); 
	}
}
