#pragma once
#include "AllHeaders.h"
#include "GraphicPipeline.h"
#include "SwapChain.h"

namespace LavaCake {
	namespace Framework {
		
		enum RenderPassFlag {
			SHOW_ON_SCREEN	= 1,
			USE_COLOR				= 2,
			USE_DEPTH				= 4,
			OP_STORE_COLOR	= 8,
			OP_STORE_DEPTH  = 16,
			ADD_INPUT				= 32
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
			RENDERPASS_COLOR_ATTACHMENT, RENDERPASS_DEPTH_ATTACHMENT, RENDERPASS_STENCIL_ATTACHMENT, RENDERPASS_INPUT_ATTACHMENT
		};

		
		class RenderPass {

			struct SubpassParameters {
				VkPipelineBindPoint                  PipelineType;
				std::vector<VkAttachmentReference>   InputAttachments;
				std::vector<VkAttachmentReference>   ColorAttachments;
				std::vector<VkAttachmentReference>   ResolveAttachments;
				VkAttachmentReference const* DepthStencilAttachment;
				std::vector<uint32_t>                PreserveAttachments;
			};

		public :

			/*
			* renderPass Constructor for an image with the swapchain format
			*/
			RenderPass();


			/*
			* renderPass Constructor for a specific image format and depth format
			*/
			RenderPass( VkFormat ImageFormat, VkFormat DepthFormat);

			/*
			* add dependencies for the render pass
			*/
			void addDependencies(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcPipe, VkPipelineStageFlags dstPipe, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkDependencyFlags dependency);

			/*
			* add a subpass composed of multiple graphics pipeline and setup their attachments 
			*/
			void addSubPass(std::vector<GraphicPipeline*> p, SubpassAttachment AttachementDescription, std::vector<uint32_t> input_number = {});

			/*
			* prepare the render pass for drawing 
			*/
			void compile();

			void reloadShaders();

			/*
			* Draw the render pass using a specific command buffer into a framebuffer
			*/
			void draw(CommandBuffer& commandBuffer, FrameBuffer& frameBuffer, vec2u viewportMin, vec2u viewportMax, std::vector<VkClearValue> const & clear_values = {{ 1.0f, 0 }});

			/*
			*	return the handle of the render pass
			*/
			VkRenderPass& getHandle();


			void prepareOutputFrameBuffer(FrameBuffer& FrameBuffer);


			void setSwapChainImage(FrameBuffer& FrameBuffer, SwapChainImage& image);


			~RenderPass() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				DestroyRenderPass(logical, m_renderPass);
			}

		private : 


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
			VkFormat																							m_imageFormat;
			VkFormat																							m_depthFormat;
			std::vector<SubpassParameters>												m_subpassParameters;
			std::vector < std::vector<GraphicPipeline*>	>					m_subpass;
			std::vector<VkAttachmentReference>										m_depthAttachments;
			std::vector<VkAttachmentDescription>									m_attachmentDescriptions;
			std::vector<VkSubpassDependency>											m_dependencies;

			std::vector<RenderPassAttachmentType>									m_attachmentype;

			std::vector<Attachment*>															m_inputAttachements;

			std::vector<std::vector<uint32_t>>										m_subpassAttachements;

			int																										m_khr_attachement = -1;

			
		};
	}
}
