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

      void prepareGui(Queue* queue, CommandBuffer* cmdBuff, SwapChainImage* img);

      void drawGui(Queue* queue, CommandBuffer* cmdBuff, SwapChainImage* img);
       
    private : 
      RenderPass* pass = nullptr;
      GraphicPipeline* pipeline = nullptr;
      PushConstant* pushConstant = nullptr;
      Framework::VertexBuffer* vertex_buffer = nullptr;
      Helpers::Mesh::Mesh mesh;
      FrameBuffer* frame = nullptr;
    };

    void prepareImput(GLFWwindow* window); 
	}
}
