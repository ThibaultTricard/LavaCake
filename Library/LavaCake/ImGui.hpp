#pragma once

#include "Device.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "GraphicPipeline.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorPool.hpp"
#include "CommandBuffer.hpp"

#include <imgui.h>

namespace LavaCake {

    /**
     * \brief Push constants structure for ImGui rendering
     */
    struct ImGuiPushConstants {
        float scale[2];      ///< Scale factors (2.0f / displayWidth, 2.0f / displayHeight)
        float translate[2];  ///< Translation (-1.0f - displayPos.x * scale.x, -1.0f - displayPos.y * scale.y)
    };

    namespace detail {
        // Embedded SPIR-V shaders (pre-compiled from imgui.vert and imgui.frag)

        // Vertex shader SPIR-V
        static constexpr uint32_t imgui_vert_spv[] = {
            0x07230203, 0x00010000, 0x000d000b, 0x0000002e, 0x00000000, 0x00020011,
            0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
            0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000b000f, 0x00000000,
            0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x0000000f,
            0x00000011, 0x00000018, 0x0000001b, 0x00030003, 0x00000002, 0x000001c2,
            0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
            0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
            0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
            0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x00000009,
            0x67617266, 0x43786554, 0x64726f6f, 0x00000000, 0x00050005, 0x0000000b,
            0x65546e69, 0x6f6f4378, 0x00006472, 0x00050005, 0x0000000f, 0x67617266,
            0x6f6c6f43, 0x00000072, 0x00040005, 0x00000011, 0x6f436e69, 0x00726f6c,
            0x00060005, 0x00000016, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000,
            0x00060006, 0x00000016, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69,
            0x00070006, 0x00000016, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953,
            0x00000000, 0x00070006, 0x00000016, 0x00000002, 0x435f6c67, 0x4470696c,
            0x61747369, 0x0065636e, 0x00070006, 0x00000016, 0x00000003, 0x435f6c67,
            0x446c6c75, 0x61747369, 0x0065636e, 0x00030005, 0x00000018, 0x00000000,
            0x00050005, 0x0000001b, 0x6f506e69, 0x69746973, 0x00006e6f, 0x00060005,
            0x0000001d, 0x68737550, 0x736e6f43, 0x746e6174, 0x00000073, 0x00050006,
            0x0000001d, 0x00000000, 0x6c616373, 0x00000065, 0x00060006, 0x0000001d,
            0x00000001, 0x6e617274, 0x74616c73, 0x00000065, 0x00030005, 0x0000001f,
            0x00006370, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047,
            0x0000000b, 0x0000001e, 0x00000001, 0x00040047, 0x0000000f, 0x0000001e,
            0x00000001, 0x00040047, 0x00000011, 0x0000001e, 0x00000002, 0x00030047,
            0x00000016, 0x00000002, 0x00050048, 0x00000016, 0x00000000, 0x0000000b,
            0x00000000, 0x00050048, 0x00000016, 0x00000001, 0x0000000b, 0x00000001,
            0x00050048, 0x00000016, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
            0x00000016, 0x00000003, 0x0000000b, 0x00000004, 0x00040047, 0x0000001b,
            0x0000001e, 0x00000000, 0x00030047, 0x0000001d, 0x00000002, 0x00050048,
            0x0000001d, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001d,
            0x00000001, 0x00000023, 0x00000008, 0x00020013, 0x00000002, 0x00030021,
            0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
            0x00000007, 0x00000006, 0x00000002, 0x00040020, 0x00000008, 0x00000003,
            0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020,
            0x0000000a, 0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b,
            0x00000001, 0x00040017, 0x0000000d, 0x00000006, 0x00000004, 0x00040020,
            0x0000000e, 0x00000003, 0x0000000d, 0x0004003b, 0x0000000e, 0x0000000f,
            0x00000003, 0x00040020, 0x00000010, 0x00000001, 0x0000000d, 0x0004003b,
            0x00000010, 0x00000011, 0x00000001, 0x00040015, 0x00000013, 0x00000020,
            0x00000000, 0x0004002b, 0x00000013, 0x00000014, 0x00000001, 0x0004001c,
            0x00000015, 0x00000006, 0x00000014, 0x0006001e, 0x00000016, 0x0000000d,
            0x00000006, 0x00000015, 0x00000015, 0x00040020, 0x00000017, 0x00000003,
            0x00000016, 0x0004003b, 0x00000017, 0x00000018, 0x00000003, 0x00040015,
            0x00000019, 0x00000020, 0x00000001, 0x0004002b, 0x00000019, 0x0000001a,
            0x00000000, 0x0004003b, 0x0000000a, 0x0000001b, 0x00000001, 0x0004001e,
            0x0000001d, 0x00000007, 0x00000007, 0x00040020, 0x0000001e, 0x00000009,
            0x0000001d, 0x0004003b, 0x0000001e, 0x0000001f, 0x00000009, 0x00040020,
            0x00000020, 0x00000009, 0x00000007, 0x0004002b, 0x00000019, 0x00000024,
            0x00000001, 0x0004002b, 0x00000006, 0x00000028, 0x00000000, 0x0004002b,
            0x00000006, 0x00000029, 0x3f800000, 0x00050036, 0x00000002, 0x00000004,
            0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007,
            0x0000000c, 0x0000000b, 0x0003003e, 0x00000009, 0x0000000c, 0x0004003d,
            0x0000000d, 0x00000012, 0x00000011, 0x0003003e, 0x0000000f, 0x00000012,
            0x0004003d, 0x00000007, 0x0000001c, 0x0000001b, 0x00050041, 0x00000020,
            0x00000021, 0x0000001f, 0x0000001a, 0x0004003d, 0x00000007, 0x00000022,
            0x00000021, 0x00050085, 0x00000007, 0x00000023, 0x0000001c, 0x00000022,
            0x00050041, 0x00000020, 0x00000025, 0x0000001f, 0x00000024, 0x0004003d,
            0x00000007, 0x00000026, 0x00000025, 0x00050081, 0x00000007, 0x00000027,
            0x00000023, 0x00000026, 0x00050051, 0x00000006, 0x0000002a, 0x00000027,
            0x00000000, 0x00050051, 0x00000006, 0x0000002b, 0x00000027, 0x00000001,
            0x00070050, 0x0000000d, 0x0000002c, 0x0000002a, 0x0000002b, 0x00000028,
            0x00000029, 0x00050041, 0x0000000e, 0x0000002d, 0x00000018, 0x0000001a,
            0x0003003e, 0x0000002d, 0x0000002c, 0x000100fd, 0x00010038
        };
        static constexpr size_t imgui_vert_spv_size = sizeof(imgui_vert_spv);

        // Fragment shader SPIR-V
        static constexpr uint32_t imgui_frag_spv[] = {
            0x07230203, 0x00010000, 0x000d000b, 0x00000018, 0x00000000, 0x00020011,
            0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
            0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004,
            0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00000014,
            0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2,
            0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
            0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
            0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
            0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
            0x4374756f, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000000b, 0x67617266,
            0x6f6c6f43, 0x00000072, 0x00050005, 0x00000010, 0x746e6f66, 0x74786554,
            0x00657275, 0x00060005, 0x00000014, 0x67617266, 0x43786554, 0x64726f6f,
            0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047,
            0x0000000b, 0x0000001e, 0x00000001, 0x00040047, 0x00000010, 0x00000021,
            0x00000000, 0x00040047, 0x00000010, 0x00000022, 0x00000000, 0x00040047,
            0x00000014, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021,
            0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
            0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
            0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020,
            0x0000000a, 0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b,
            0x00000001, 0x00090019, 0x0000000d, 0x00000006, 0x00000001, 0x00000000,
            0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x0000000e,
            0x0000000d, 0x00040020, 0x0000000f, 0x00000000, 0x0000000e, 0x0004003b,
            0x0000000f, 0x00000010, 0x00000000, 0x00040017, 0x00000012, 0x00000006,
            0x00000002, 0x00040020, 0x00000013, 0x00000001, 0x00000012, 0x0004003b,
            0x00000013, 0x00000014, 0x00000001, 0x00050036, 0x00000002, 0x00000004,
            0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007,
            0x0000000c, 0x0000000b, 0x0004003d, 0x0000000e, 0x00000011, 0x00000010,
            0x0004003d, 0x00000012, 0x00000015, 0x00000014, 0x00050057, 0x00000007,
            0x00000016, 0x00000011, 0x00000015, 0x00050085, 0x00000007, 0x00000017,
            0x0000000c, 0x00000016, 0x0003003e, 0x00000009, 0x00000017, 0x000100fd,
            0x00010038
        };
        static constexpr size_t imgui_frag_spv_size = sizeof(imgui_frag_spv);
    }

    /**
     * \brief ImGui renderer class for LavaCake
     *
     * This class provides ImGui integration with LavaCake's Vulkan wrapper.
     * It manages its own graphics pipeline, descriptor sets, and buffers.
     * Shaders are embedded as SPIR-V, so no external files are needed.
     *
     * Usage:
     * \code
     * #include <LavaCake/ImGui.hpp>
     * #include <imgui_impl_glfw.h>
     *
     * LavaCake::ImGuiRenderer imgui(device, device.getSwapchainFormat());
     *
     * // Initialize your platform backend
     * ImGui::SetCurrentContext(imgui.getContext());
     * ImGui_ImplGlfw_InitForVulkan(window, true);
     *
     * // In render loop:
     * ImGui_ImplGlfw_NewFrame();
     * imgui.newFrame();
     * ImGui::ShowDemoWindow();
     *
     * // Inside dynamic rendering context:
     * imgui.render(cmdBuffer);
     *
     * // Before destruction:
     * ImGui_ImplGlfw_Shutdown();
     * \endcode
     */
    class ImGuiRenderer {
    public:
        /**
         * \brief Default constructor
         */
        ImGuiRenderer() = default;

        /**
         * \brief Constructs an ImGuiRenderer
         * \param device The LavaCake device
         * \param colorFormat The swapchain color format
         */
        ImGuiRenderer(LavaCake::Device& device, vk::Format colorFormat)
            : m_device(device), m_colorFormat(colorFormat)
        {
            // Create ImGui context
            IMGUI_CHECKVERSION();
            m_imguiContext = ImGui::CreateContext();
            ImGui::SetCurrentContext(m_imguiContext);

            // Configure ImGui IO
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            // Create Vulkan resources
            createFontTexture();
            createDescriptorResources();
            createPipeline();

            // Allocate initial buffers
            m_vertexBufferCapacity = 65536;
            m_indexBufferCapacity = 65536 * 3;

            m_vertexBuffer = std::make_unique<Buffer>(
                m_device,
                m_vertexBufferCapacity * sizeof(ImDrawVert),
                vk::BufferUsageFlagBits::eVertexBuffer,
                vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite
            );

            m_indexBuffer = std::make_unique<Buffer>(
                m_device,
                m_indexBufferCapacity * sizeof(ImDrawIdx),
                vk::BufferUsageFlagBits::eIndexBuffer,
                vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite
            );

            m_initialized = true;
        }

        /**
         * \brief Destructor - cleans up ImGui context and resources
         */
        ~ImGuiRenderer() {
            cleanup();
        }

        // Delete copy constructor and assignment
        ImGuiRenderer(const ImGuiRenderer&) = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

        /**
         * \brief Move constructor
         */
        ImGuiRenderer(ImGuiRenderer&& other) noexcept
            : m_device(other.m_device)
            , m_colorFormat(other.m_colorFormat)
            , m_imguiContext(std::exchange(other.m_imguiContext, nullptr))
            , m_fontImage(std::move(other.m_fontImage))
            , m_fontImageView(std::move(other.m_fontImageView))
            , m_fontSampler(std::move(other.m_fontSampler))
            , m_pipeline(std::move(other.m_pipeline))
            , m_descriptorPool(std::move(other.m_descriptorPool))
            , m_descriptorSetLayout(std::move(other.m_descriptorSetLayout))
            , m_descriptorSet(std::exchange(other.m_descriptorSet, nullptr))
            , m_vertexBuffer(std::move(other.m_vertexBuffer))
            , m_indexBuffer(std::move(other.m_indexBuffer))
            , m_vertexBufferCapacity(std::exchange(other.m_vertexBufferCapacity, 0))
            , m_indexBufferCapacity(std::exchange(other.m_indexBufferCapacity, 0))
            , m_initialized(std::exchange(other.m_initialized, false))
        {}

        /**
         * \brief Move assignment operator
         */
        ImGuiRenderer& operator=(ImGuiRenderer&& other) noexcept {
            if (this != &other) {
                cleanup();

                m_device = other.m_device;
                m_colorFormat = other.m_colorFormat;
                m_imguiContext = std::exchange(other.m_imguiContext, nullptr);
                m_fontImage = std::move(other.m_fontImage);
                m_fontImageView = std::move(other.m_fontImageView);
                m_fontSampler = std::move(other.m_fontSampler);
                m_pipeline = std::move(other.m_pipeline);
                m_descriptorPool = std::move(other.m_descriptorPool);
                m_descriptorSetLayout = std::move(other.m_descriptorSetLayout);
                m_descriptorSet = std::exchange(other.m_descriptorSet, nullptr);
                m_vertexBuffer = std::move(other.m_vertexBuffer);
                m_indexBuffer = std::move(other.m_indexBuffer);
                m_vertexBufferCapacity = std::exchange(other.m_vertexBufferCapacity, 0);
                m_indexBufferCapacity = std::exchange(other.m_indexBufferCapacity, 0);
                m_initialized = std::exchange(other.m_initialized, false);
            }
            return *this;
        }

        /**
         * \brief Get the ImGui context for platform backend initialization
         *
         * Use this to initialize your platform backend:
         * \code
         * ImGui::SetCurrentContext(imgui.getContext());
         * ImGui_ImplGlfw_InitForVulkan(window, true);
         * \endcode
         */
        ImGuiContext* getContext() const { return m_imguiContext; }

        /**
         * \brief Start a new ImGui frame
         *
         * Call your platform backend's NewFrame() before this:
         * \code
         * ImGui_ImplGlfw_NewFrame();  // Platform-specific
         * imgui.newFrame();            // Then this
         * \endcode
         */
        void newFrame() {
            ImGui::SetCurrentContext(m_imguiContext);
            ImGui::NewFrame();
        }

        /**
         * \brief Render ImGui draw data to the command buffer
         * Call this inside your dynamic rendering context
         * \param cmd The command buffer to record into
         */
        void render(const vk::CommandBuffer& cmd) {
            ImGui::SetCurrentContext(m_imguiContext);
            ImGui::Render();

            ImDrawData* drawData = ImGui::GetDrawData();
            if (drawData->TotalVtxCount == 0) return;

            // Update buffers with draw data
            updateBuffers(drawData);

            // Setup push constants for orthographic projection
            ImGuiPushConstants pushConstants;
            pushConstants.scale[0] = 2.0f / drawData->DisplaySize.x;
            pushConstants.scale[1] = 2.0f / drawData->DisplaySize.y;
            pushConstants.translate[0] = -1.0f - drawData->DisplayPos.x * pushConstants.scale[0];
            pushConstants.translate[1] = -1.0f - drawData->DisplayPos.y * pushConstants.scale[1];

            // Bind pipeline
            m_pipeline->bind(cmd);

            // Bind descriptor set
            cmd.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                m_pipeline->getLayout(),
                0,
                {m_descriptorSet},
                {}
            );

            // Push constants
            cmd.pushConstants(
                m_pipeline->getLayout(),
                vk::ShaderStageFlagBits::eVertex,
                0,
                sizeof(ImGuiPushConstants),
                &pushConstants
            );

            // Bind vertex buffer
            vk::DeviceSize offset = 0;
            vk::Buffer vertexBuf = m_vertexBuffer->getBuffer();
            cmd.bindVertexBuffers(0, 1, &vertexBuf, &offset);

            // Bind index buffer
            cmd.bindIndexBuffer(
                m_indexBuffer->getBuffer(),
                0,
                sizeof(ImDrawIdx) == 2 ? vk::IndexType::eUint16 : vk::IndexType::eUint32
            );

            // Set viewport
            vk::Viewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = drawData->DisplaySize.x * drawData->FramebufferScale.x;
            viewport.height = drawData->DisplaySize.y * drawData->FramebufferScale.y;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            cmd.setViewport(0, 1, &viewport);

            // Render command lists
            int globalVtxOffset = 0;
            int globalIdxOffset = 0;
            ImVec2 clipOff = drawData->DisplayPos;
            ImVec2 clipScale = drawData->FramebufferScale;

            for (int n = 0; n < drawData->CmdListsCount; n++) {
                const ImDrawList* cmdList = drawData->CmdLists[n];

                for (int cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.Size; cmdIdx++) {
                    const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdIdx];

                    if (pcmd->UserCallback != nullptr) {
                        if (pcmd->UserCallback != ImDrawCallback_ResetRenderState) {
                            pcmd->UserCallback(cmdList, pcmd);
                        }
                    } else {
                        // Calculate scissor rect
                        ImVec2 clipMin(
                            (pcmd->ClipRect.x - clipOff.x) * clipScale.x,
                            (pcmd->ClipRect.y - clipOff.y) * clipScale.y
                        );
                        ImVec2 clipMax(
                            (pcmd->ClipRect.z - clipOff.x) * clipScale.x,
                            (pcmd->ClipRect.w - clipOff.y) * clipScale.y
                        );

                        // Clamp to framebuffer
                        if (clipMin.x < 0.0f) clipMin.x = 0.0f;
                        if (clipMin.y < 0.0f) clipMin.y = 0.0f;
                        if (clipMax.x > viewport.width) clipMax.x = viewport.width;
                        if (clipMax.y > viewport.height) clipMax.y = viewport.height;
                        if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) continue;

                        // Set scissor
                        vk::Rect2D scissor;
                        scissor.offset.x = static_cast<int32_t>(clipMin.x);
                        scissor.offset.y = static_cast<int32_t>(clipMin.y);
                        scissor.extent.width = static_cast<uint32_t>(clipMax.x - clipMin.x);
                        scissor.extent.height = static_cast<uint32_t>(clipMax.y - clipMin.y);
                        cmd.setScissor(0, 1, &scissor);

                        // Draw
                        cmd.drawIndexed(
                            pcmd->ElemCount,
                            1,
                            pcmd->IdxOffset + globalIdxOffset,
                            pcmd->VtxOffset + globalVtxOffset,
                            0
                        );
                    }
                }

                globalIdxOffset += cmdList->IdxBuffer.Size;
                globalVtxOffset += cmdList->VtxBuffer.Size;
            }
        }

        /**
         * \brief Check if the renderer is initialized
         * \return true if initialized
         */
        bool isInitialized() const { return m_initialized; }

    private:
        LavaCake::Device m_device;
        vk::Format m_colorFormat = vk::Format::eUndefined;

        // ImGui context
        ImGuiContext* m_imguiContext = nullptr;

        // Font atlas texture
        std::unique_ptr<Image> m_fontImage;
        std::unique_ptr<ImageView> m_fontImageView;
        std::unique_ptr<Sampler> m_fontSampler;

        // Graphics pipeline
        std::unique_ptr<GraphicsPipeline> m_pipeline;

        // Descriptor management (internal pool)
        std::unique_ptr<DescriptorPool> m_descriptorPool;
        std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
        vk::DescriptorSet m_descriptorSet;

        // Vertex and index buffers (dynamic sizing)
        std::unique_ptr<Buffer> m_vertexBuffer;
        std::unique_ptr<Buffer> m_indexBuffer;
        uint32_t m_vertexBufferCapacity = 0;
        uint32_t m_indexBufferCapacity = 0;

        // Initialization state
        bool m_initialized = false;

        /**
         * \brief Create the font texture from ImGui font atlas
         */
        void createFontTexture() {
            ImGuiIO& io = ImGui::GetIO();

            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

            std::vector<unsigned char> imageData(pixels, pixels + width * height * 4);

            m_fontImage = std::make_unique<Image>(
                m_device,
                imageData,
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                1,
                vk::Format::eR8G8B8A8Unorm,
                vk::ImageUsageFlagBits::eSampled
            );

            m_fontImageView = std::make_unique<ImageView>(*m_fontImage);

            vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter = vk::Filter::eLinear;
            samplerInfo.minFilter = vk::Filter::eLinear;
            samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
            samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
            samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
            samplerInfo.minLod = -1000.0f;
            samplerInfo.maxLod = 1000.0f;
            samplerInfo.anisotropyEnable = VK_FALSE;

            m_fontSampler = std::make_unique<Sampler>(m_device, samplerInfo);
        }

        /**
         * \brief Create descriptor pool and set for font texture
         */
        void createDescriptorResources() {
            m_descriptorSetLayout = std::make_unique<DescriptorSetLayout>(
                DescriptorSetLayout::Builder(m_device)
                    .addCombinedImageSampler(0, vk::ShaderStageFlagBits::eFragment)
                    .build()
            );

            m_descriptorPool = std::make_unique<DescriptorPool>(
                DescriptorPool::Builder(m_device)
                    .addCombinedImageSamplers(1)
                    .setMaxSets(1)
                    .build()
            );

            m_descriptorSet = m_descriptorPool->allocate(m_descriptorSetLayout->getLayout());

            DescriptorSetUpdater(m_device, m_descriptorSet)
                .bindImage(0, m_fontImageView->getImageView(), m_fontSampler->getSampler())
                .update();

            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(m_descriptorSet)));
        }

        /**
         * \brief Create the graphics pipeline for ImGui rendering
         */
        void createPipeline() {
            // Configure vertex input for ImDrawVert
            GraphicsPipeline::VertexInputInfo vertexInputInfo;

            vertexInputInfo.bindings.push_back(vk::VertexInputBindingDescription{
                0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex
            });

            vertexInputInfo.attributes.push_back(vk::VertexInputAttributeDescription{
                0, 0, vk::Format::eR32G32Sfloat, static_cast<uint32_t>(offsetof(ImDrawVert, pos))
            });
            vertexInputInfo.attributes.push_back(vk::VertexInputAttributeDescription{
                1, 0, vk::Format::eR32G32Sfloat, static_cast<uint32_t>(offsetof(ImDrawVert, uv))
            });
            vertexInputInfo.attributes.push_back(vk::VertexInputAttributeDescription{
                2, 0, vk::Format::eR8G8B8A8Unorm, static_cast<uint32_t>(offsetof(ImDrawVert, col))
            });

            // Alpha blending configuration
            vk::PipelineColorBlendAttachmentState blendAttachment{};
            blendAttachment.blendEnable = VK_TRUE;
            blendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            blendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            blendAttachment.colorBlendOp = vk::BlendOp::eAdd;
            blendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            blendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            blendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
            blendAttachment.colorWriteMask =
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

            // Build pipeline using embedded SPIR-V shaders
            m_pipeline = std::make_unique<GraphicsPipeline>(
                GraphicsPipeline::Builder(m_device)
                    .addShaderFromSpirvByteCode(detail::imgui_vert_spv,
                                                detail::imgui_vert_spv_size,
                                                vk::ShaderStageFlagBits::eVertex)
                    .addShaderFromSpirvByteCode(detail::imgui_frag_spv,
                                                detail::imgui_frag_spv_size,
                                                vk::ShaderStageFlagBits::eFragment)
                    .addColorAttachmentFormat(m_colorFormat)
                    .setVertexInput(vertexInputInfo)
                    .setTopology(vk::PrimitiveTopology::eTriangleList)
                    .setCullMode(vk::CullModeFlagBits::eNone)
                    .setDepthTest(false, false)
                    .setColorBlendAttachments({blendAttachment})
                    .addDescriptorSetLayout(m_descriptorSetLayout->getLayout())
                    .addPushConstantRange(
                        vk::ShaderStageFlagBits::eVertex,
                        0,
                        sizeof(ImGuiPushConstants)
                    )
                    .build()
            );
        }

        /**
         * \brief Ensure buffers can hold the required vertex/index count
         */
        void ensureBufferCapacity(uint32_t vertexCount, uint32_t indexCount) {
            if (vertexCount > m_vertexBufferCapacity) {
                uint32_t newCapacity = vertexCount + 5000;
                m_vertexBuffer = std::make_unique<Buffer>(
                    m_device,
                    newCapacity * sizeof(ImDrawVert),
                    vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite
                );
                m_vertexBufferCapacity = newCapacity;
            }

            if (indexCount > m_indexBufferCapacity) {
                uint32_t newCapacity = indexCount + 10000;
                m_indexBuffer = std::make_unique<Buffer>(
                    m_device,
                    newCapacity * sizeof(ImDrawIdx),
                    vk::BufferUsageFlagBits::eIndexBuffer,
                    vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite
                );
                m_indexBufferCapacity = newCapacity;
            }
        }

        /**
         * \brief Update buffers with ImGui draw data
         */
        void updateBuffers(ImDrawData* drawData) {
            if (drawData->TotalVtxCount == 0) return;

            ensureBufferCapacity(
                static_cast<uint32_t>(drawData->TotalVtxCount),
                static_cast<uint32_t>(drawData->TotalIdxCount)
            );

            ImDrawVert* vtxDst = static_cast<ImDrawVert*>(m_vertexBuffer->map());
            ImDrawIdx* idxDst = static_cast<ImDrawIdx*>(m_indexBuffer->map());

            for (int n = 0; n < drawData->CmdListsCount; n++) {
                const ImDrawList* cmdList = drawData->CmdLists[n];
                memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtxDst += cmdList->VtxBuffer.Size;
                idxDst += cmdList->IdxBuffer.Size;
            }

            m_vertexBuffer->unmap();
            m_indexBuffer->unmap();
        }

        /**
         * \brief Clean up resources
         *
         * Note: You must call your platform backend's shutdown before destroying
         * the ImGuiRenderer (e.g., ImGui_ImplGlfw_Shutdown())
         */
        void cleanup() {
            if (m_imguiContext) {
                ImGui::DestroyContext(m_imguiContext);
                m_imguiContext = nullptr;
            }
            m_initialized = false;
        }
    };

} // namespace LavaCake
