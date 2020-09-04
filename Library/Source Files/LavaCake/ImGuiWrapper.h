#pragma once

#include "lib/imgui/imgui.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "RenderPass.h"
#include "Window.h"

namespace LavaCake {
	namespace Framework {

    class ImGuiWrapper {
    public : 
      ImGuiWrapper() {};

      void initGui(Window* win, Queue* queue, CommandBuffer* cmdBuff);

      void prepareGui(Queue* queue, CommandBuffer* cmdBuff);

       

      GraphicPipeline* getPipeline() {
        return pipeline;
      }
    private : 
      GraphicPipeline* pipeline = nullptr;
      PushConstant* pushConstant = nullptr;
      Framework::VertexBuffer* vertex_buffer = nullptr;
      Helpers::Mesh::Mesh mesh;


      
    };

    static GLFWmousebuttonfun   s_PrevUserCallbackMousebutton = NULL;
    static GLFWscrollfun        s_PrevUserCallbackScroll = NULL;
    static GLFWkeyfun           s_PrevUserCallbackKey = NULL;
    static GLFWcharfun          s_PrevUserCallbackChar = NULL;
    static GLFWcursorposfun     s_PrevUserCallbackMouseMotion = NULL;
    void prepareImput(GLFWwindow* window); 
	}
}
