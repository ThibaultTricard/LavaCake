#include "ImGuiWrapper.h"
#include <stdio.h>



namespace LavaCake {
  namespace Framework {

    static void ImGui_ImplGlfw_Mouse_Position(GLFWwindow* window, double xpos, double ypos) {
      ImGuiIO& io = ImGui::GetIO();
      io.MousePos = ImVec2((float)xpos, (float)ypos);
      if (s_PrevUserCallbackMousebutton!= NULL) {
        s_PrevUserCallbackMouseMotion(window, xpos, ypos);
      }
    }
    static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
    {
      return glfwGetClipboardString((GLFWwindow*)user_data);
    }

    static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
    {
      glfwSetClipboardString((GLFWwindow*)user_data, text);
    }

    void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {

      if (action == GLFW_PRESS && button >= 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[button] = true;
      }
      if (action == GLFW_RELEASE && button >= 0) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[button] = false;
      }
      if (s_PrevUserCallbackMousebutton != NULL) {
        s_PrevUserCallbackMousebutton(window, button, action, mods);
      }
    }

    void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
      ImGuiIO& io = ImGui::GetIO();
      io.MouseWheelH += (float)xoffset;
      io.MouseWheel += (float)yoffset;
      if (s_PrevUserCallbackScroll != NULL) {
        s_PrevUserCallbackScroll(window, xoffset, yoffset);
      }
    }

    void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
      ImGuiIO& io = ImGui::GetIO();
      if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
      if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

      // Modifiers are not reliable across systems
      io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
      io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
      io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
#ifdef _WIN32
      io.KeySuper = false;
#else
      io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
#endif
      if (s_PrevUserCallbackKey != NULL) {
        s_PrevUserCallbackKey(window,  key,  scancode,  action,  mods);
      }
    }

    void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
    {
      ImGuiIO& io = ImGui::GetIO();
      io.AddInputCharacter(c);

      if (s_PrevUserCallbackChar != NULL) {
        s_PrevUserCallbackChar(window,c);
      }
    }

    // glsl_shader.vert, compiled with:
    // # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
    /*
    #version 450 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aUV;
    layout(location = 2) in vec4 aColor;
    layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

    out gl_PerVertex { vec4 gl_Position; };
    layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

    void main()
    {
      Out.Color = aColor;
      Out.UV = aUV;
      gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
    }
    */



    static const uint32_t __glsl_shader_vert_spv[] =
    {
        0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
        0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
        0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
        0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
        0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
        0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
        0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
        0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
        0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
        0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
        0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
        0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
        0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
        0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
        0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
        0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
        0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
        0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
        0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
        0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
        0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
        0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
        0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
        0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
        0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
        0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
        0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
        0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
        0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
        0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
        0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
        0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
        0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
        0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
        0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
        0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
        0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
        0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
        0x0000002d,0x0000002c,0x000100fd,0x00010038
    };

    // glsl_shader.frag, compiled with:
    // # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
    /*
    #version 450 core
    layout(location = 0) out vec4 fColor;
    layout(set=0, binding=0) uniform sampler2D sTexture;
    layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
    void main()
    {
        fColor = In.Color * texture(sTexture, In.UV.st);
    }
    */
    static uint32_t __glsl_shader_frag_spv[] =
    {
        0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
        0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
        0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
        0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
        0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
        0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
        0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
        0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
        0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
        0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
        0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
        0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
        0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
        0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
        0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
        0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
        0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
        0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
        0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
        0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
        0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
        0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
        0x00010038
    };

    void ImGuiWrapper::initGui(Window* win, Queue* queue, CommandBuffer* cmdBuff) {
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGui::StyleColorsDark();
      prepareInput(win->m_window);
      ImGuiIO& io = ImGui::GetIO();
     
      // Setup display size (every frame to accommodate for window resizing)
      int Wwidth, Wheight;
      int display_w, display_h;
      glfwGetWindowSize(win->m_window, &Wwidth, &Wheight);
      glfwGetFramebufferSize(win->m_window, &display_w, &display_h);
      io.DisplaySize = ImVec2((float)Wwidth, (float)Wheight);
      if (Wwidth > 0 && Wheight > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / Wwidth, (float)display_h / Wheight);

      Framework::Device* d = Framework::Device::getDevice();
      SwapChain* s = SwapChain::getSwapChain();
      VkExtent2D size = s->size();
      VkDevice logical = d->getLogicalDevice();

      unsigned char* pixels;
      int width, height;
      io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
      std::vector<unsigned char>* textureData = new std::vector<unsigned char>(pixels, pixels + width * height * 4);



      m_fontBuffer = Framework::createTextureBuffer(queue, *cmdBuff, textureData, width, height,1, 4);


      m_mesh = (Geometry::Mesh_t*) (new Geometry::IndexedMesh<Geometry::TRIANGLE>(imguiformat));


      m_mesh->appendVertex({
          -1.0f, -1.0f,
           0.0f, 0.0f,
           0.0f,  0.0f, 0.0f, 0.0f });
      m_mesh->appendVertex({
          -1.0f,  1.0f,
           0.0f, 1.0f,
           0.0f,  0.0f, 0.0f, 0.0f });
      m_mesh->appendVertex({
           1.0f, -1.0f,
           1.0f, 0.0f,
           0.0f,  0.0f, 0.0f, 0.0f });

      m_mesh->appendIndex(0);
      m_mesh->appendIndex(1);
      m_mesh->appendIndex(2);



      m_vertexBuffer = new Framework::VertexBuffer({ m_mesh });
      m_vertexBuffer->allocate(queue, *cmdBuff);

      m_pushConstant = new PushConstant();
      vec2f scale = vec2f({0.0f,0.0f});
      vec2f translate = vec2f({0.0f,0.0f});
      m_pushConstant->addVariable("uScale", scale);
      m_pushConstant->addVariable("uTranslate", translate);

      std::vector<unsigned char>	vertSpirv(sizeof(__glsl_shader_vert_spv) / sizeof(unsigned char));
      memcpy(&vertSpirv[0], __glsl_shader_vert_spv, sizeof(__glsl_shader_vert_spv));
      std::vector<unsigned char>	fragSpirv(sizeof(__glsl_shader_frag_spv) / sizeof(unsigned char));
      memcpy(&fragSpirv[0], __glsl_shader_frag_spv, sizeof(__glsl_shader_frag_spv));

      m_pipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
      VertexShaderModule* sphereVertex = new Framework::VertexShaderModule(vertSpirv);
      m_pipeline->setVertexModule(sphereVertex);

      FragmentShaderModule* sphereFrag = new Framework::FragmentShaderModule(fragSpirv);
      m_pipeline->setFragmentModule(sphereFrag);
      m_pipeline->setVerticesInfo(m_vertexBuffer->getBindingDescriptions(), m_vertexBuffer->getAttributeDescriptions(), m_vertexBuffer->primitiveTopology());
      m_pipeline->setVertices({ m_vertexBuffer });

      m_pipeline->addPushContant(m_pushConstant, VK_SHADER_STAGE_VERTEX_BIT);
      m_pipeline->addTextureBuffer(m_fontBuffer, VK_SHADER_STAGE_FRAGMENT_BIT,0);
      m_pipeline->SetCullMode(VK_CULL_MODE_NONE);
      m_pipeline->setAlphaBlending(true);

    }


    

    
    void ImGuiWrapper::prepareGui(Queue* queue, CommandBuffer& cmdBuff) {


      SwapChain* s = SwapChain::getSwapChain();
      VkExtent2D size = s->size();
      ImGui::Render();
      ImDrawData* draw_data = ImGui::GetDrawData();

      size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert)/sizeof(float);
      size_t index_size = draw_data->TotalIdxCount ;

			m_mesh->vertices().clear();
			m_mesh->indices().clear();

      uint32_t offset = 0;
      for (int n = 0; n < draw_data->CmdListsCount; n++)
      {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int j = 0; j < cmd_list->VtxBuffer.Size; j++) {
          float x = cmd_list->VtxBuffer.Data[j].pos[0];
          float y = cmd_list->VtxBuffer.Data[j].pos[1];
          float u = cmd_list->VtxBuffer.Data[j].uv[0];
          float v = cmd_list->VtxBuffer.Data[j].uv[1];
          float a = float(static_cast<uint8_t>((cmd_list->VtxBuffer.Data[j].col & 0xFF000000) >> 24)) /255.0f;
          float r = float(static_cast<uint8_t>((cmd_list->VtxBuffer.Data[j].col & 0x00FF0000) >> 16)) / 255.0f;
          float g = float(static_cast<uint8_t>((cmd_list->VtxBuffer.Data[j].col & 0x0000FF00) >> 8)) / 255.0f;
          float b = float(static_cast<uint8_t>(cmd_list->VtxBuffer.Data[j].col & 0x000000FF)) / 255.0f;
          m_mesh->appendVertex({ x,y,u,v,b,g,r,a });
        }
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++) {
          m_mesh->appendIndex(static_cast<uint32_t>(cmd_list->IdxBuffer.Data[j] + offset));
        }
        offset = uint32_t(m_mesh->vertices().size()/8) ;
      }

			

      m_vertexBuffer->swapMeshes({ m_mesh });
      m_vertexBuffer->allocate(queue, cmdBuff);

      vec2f scale = vec2f({ 2.0f / draw_data->DisplaySize.x , 2.0f / draw_data->DisplaySize.y });
      vec2f translate = vec2f({ -1.0f - draw_data->DisplayPos.x * scale[0] , -1.0f - draw_data->DisplayPos.y * scale[1] });
      m_pushConstant->setVariable("uScale", scale);
      m_pushConstant->setVariable("uTranslate", translate);

    }


    void prepareInput(GLFWwindow* window) {

      // Setup back-end capabilities flags
      ImGuiIO& io = ImGui::GetIO();
      io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
      io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
      io.BackendPlatformName = "imgui_impl_glfw";

      io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
      io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
      io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
      io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
      io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
      io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
      io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
      io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
      io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
      io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
      io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
      io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
      io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
      io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
      io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
      io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
      io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
      io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
      io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
      io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
      io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
      io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;


#if defined(_WIN32)
      io.ImeWindowHandle = (void*)glfwGetWin32Window(window);
#endif
      GLFWerrorfun prev_error_callback = glfwSetErrorCallback(NULL);

      s_PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
      s_PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
      s_PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
      s_PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
      s_PrevUserCallbackMouseMotion = glfwSetCursorPosCallback(window, ImGui_ImplGlfw_Mouse_Position);
         
    }


    void ImGuiWrapper::resizeGui(Window* win) {
      
      Framework::Device* d = Framework::Device::getDevice();
      SwapChain* s = SwapChain::getSwapChain();
      VkExtent2D size = s->size();
      VkDevice logical = d->getLogicalDevice();

      ImGuiIO& io = ImGui::GetIO();
      int Wwidth, Wheight;
      int display_w, display_h;
      glfwGetWindowSize(win->m_window, &Wwidth, &Wheight);
      glfwGetFramebufferSize(win->m_window, &display_w, &display_h);
      io.DisplaySize = ImVec2((float)Wwidth, (float)Wheight);
      if (Wwidth > 0 && Wheight > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / Wwidth, (float)display_h / Wheight);


      std::vector<unsigned char>	vertSpirv(sizeof(__glsl_shader_vert_spv) / sizeof(unsigned char));
      memcpy(&vertSpirv[0], __glsl_shader_vert_spv, sizeof(__glsl_shader_vert_spv));
      std::vector<unsigned char>	fragSpirv(sizeof(__glsl_shader_frag_spv) / sizeof(unsigned char));
      memcpy(&fragSpirv[0], __glsl_shader_frag_spv, sizeof(__glsl_shader_frag_spv));

      m_pipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
      VertexShaderModule* sphereVertex = new Framework::VertexShaderModule(vertSpirv);
      m_pipeline->setVertexModule(sphereVertex);

      FragmentShaderModule* sphereFrag = new Framework::FragmentShaderModule(fragSpirv);
      m_pipeline->setFragmentModule(sphereFrag);
      m_pipeline->setVerticesInfo(m_vertexBuffer->getBindingDescriptions(), m_vertexBuffer->getAttributeDescriptions(), m_vertexBuffer->primitiveTopology());
      m_pipeline->setVertices({ m_vertexBuffer });

      m_pipeline->addPushContant(m_pushConstant, VK_SHADER_STAGE_VERTEX_BIT);
      m_pipeline->addTextureBuffer(m_fontBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
      m_pipeline->SetCullMode(VK_CULL_MODE_NONE);
      m_pipeline->setAlphaBlending(true);
    }

  }
}
