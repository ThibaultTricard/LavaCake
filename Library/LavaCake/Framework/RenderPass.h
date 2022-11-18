#pragma once
#include "AllHeaders.h"
#include "GraphicPipeline.h"
#include "SwapChain.h"

namespace LavaCake {
  namespace Framework {

    enum RenderPassFlag {
      SHOW_ON_SCREEN = 1,
      USE_COLOR = 2,
      USE_DEPTH = 4,
      OP_STORE_COLOR = 8,
      OP_STORE_DEPTH = 16,
      ADD_INPUT = 32
    };

    struct SubpassAttachment {
      uint16_t nbColor = 0;
      bool storeColor = false;

      bool useDepth = false;
      bool storeDepth = false;

      bool addInput = false;

      bool showOnScreen = false;
      uint16_t showOnScreenIndex = 0;
    };


    inline RenderPassFlag operator|(RenderPassFlag a, RenderPassFlag b)
    {
      return static_cast<RenderPassFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }


    enum RenderPassAttachmentType {
      RENDERPASS_UNDEFINED_ATTACHMENT = 0,
      RENDERPASS_COLOR_ATTACHMENT = 1,
      RENDERPASS_DEPTH_ATTACHMENT = 2,
      RENDERPASS_STENCIL_ATTACHMENT = 4,
      RENDERPASS_INPUT_ATTACHMENT = 8,

    };

    inline RenderPassAttachmentType operator|(RenderPassAttachmentType a, RenderPassAttachmentType b)
    {
      return static_cast<RenderPassAttachmentType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }


    RenderPassAttachmentType toAttachmentType(VkFormat format);

    class RenderPass {

      using SubPass = std::vector< std::shared_ptr<GraphicPipeline> >;

      struct SubpassParameters {
        VkPipelineBindPoint                  PipelineType;
        std::vector<VkAttachmentReference>   InputAttachments;
        std::vector<VkAttachmentReference>   ColorAttachments;
        std::vector<VkAttachmentReference>   ResolveAttachments;
        VkAttachmentReference const* DepthStencilAttachment;
        std::vector<uint32_t>                PreserveAttachments;
      };

    public:

      /**
      \brief RenderPass constructor
      */
      RenderPass();


      /**
      \brief RenderPass constructor
      \param ImageFormat 
      */
      RenderPass(VkFormat ImageFormat, VkFormat DepthFormat);

      /*
      \brief add dependencies for the render pass
      */
      void addDependencies(
        uint32_t srcSubpass, 
        uint32_t dstSubpass, 
        VkPipelineStageFlags srcPipe, 
        VkPipelineStageFlags dstPipe, 
        VkAccessFlags srcAccess, 
        VkAccessFlags dstAccess, 
        VkDependencyFlags dependency);

      /*
      \brief add a subpass composed of multiple graphics pipeline and setup their attachments
      */
      uint32_t addSubPass(SubpassAttachment AttachementDescription, std::vector<uint32_t> input_number = {});

      /*
      \brief Compile the renderpass
      */
      void compile();


      /*
      \brief start the drawing of the RenderPass
      */
      void begin(CommandBuffer& cmdBuff, FrameBuffer& frameBuffer, vec2u viewportMin, vec2u viewportMax, std::vector<VkClearValue> const& clear_values);

      /*
      \brief start the drawing of the RenderPass
      */
      void nextSubPass(CommandBuffer& cmdBuff);

      /*
      \brief end the drawing of the RenderPass
      */
      void end(CommandBuffer& cmdBuff);

      /*
      \brief Register a draw call in a command buffer
      */
      //void draw(CommandBuffer& commandBuffer, FrameBuffer& frameBuffer, vec2u viewportMin, vec2u viewportMax, std::vector<VkClearValue> const& clear_values = { { 1.0f, 0 } });

      /*
      \return the handle of the render pass
      */
      const VkRenderPass& getHandle() const;


      void prepareOutputFrameBuffer(const Queue& queue, CommandBuffer& commandBuffer, FrameBuffer& FrameBuffer);


      void setSwapChainImage(FrameBuffer& FrameBuffer, const SwapChainImage& image);


      ~RenderPass() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        DestroyRenderPass(logical, m_renderPass);
      }

    private:


      /*
      * add an attachment for a subpass
      */
      void addAttatchments(SubpassAttachment AttachementDescription, std::vector<uint32_t> input_number = {});

      void SpecifySubpassDescriptions(std::vector<SubpassParameters> const& subpass_parameters,
        std::vector<VkSubpassDescription>& subpass_descriptions);

      bool CreateRenderPass(VkDevice                                     logical_device,
        std::vector<VkAttachmentDescription> const& attachments_descriptions,
        std::vector<SubpassParameters> const& subpass_parameters,
        std::vector<VkSubpassDependency> const& subpass_dependencies,
        VkRenderPass& render_pass);

      void DestroyRenderPass(VkDevice       logical_device,
        VkRenderPass& render_pass);

      VkRenderPass															            m_renderPass = VK_NULL_HANDLE;
      VkFormat																							m_imageFormat = VK_FORMAT_UNDEFINED;
      VkFormat																							m_depthFormat = VK_FORMAT_UNDEFINED;
      std::vector<SubpassParameters>												m_subpassParameters;
      //std::vector < SubPass >																m_subpass;
      std::vector<VkAttachmentReference>										m_depthAttachments;
      std::vector<VkAttachmentDescription>									m_attachmentDescriptions;
      std::vector<VkSubpassDependency>											m_dependencies;

      std::vector<RenderPassAttachmentType>									m_attachmentype;

      std::vector<std::shared_ptr<Image>>										m_inputAttachements;

      std::vector<std::vector<uint32_t>>										m_subpassAttachements;

      int																										m_khr_attachement = -1;


    };
  }
}
