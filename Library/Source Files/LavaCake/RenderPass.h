#pragma once
#include "AllHeaders.h"
#include "LavaCake/GraphicPipeline.h"

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
		

		inline RenderPassFlag operator|(RenderPassFlag a, RenderPassFlag b)
		{
			return static_cast<RenderPassFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
		}

		class RenderPass {
		public :


			RenderPass();

			RenderPass( VkFormat ImageFormat, VkFormat DepthFormat);

			void addAttatchments(uint32_t AttachementFlag, std::vector<uint32_t> input_number = {});

			void addDependencies(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcPipe, VkPipelineStageFlags dstPipe, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkDependencyFlags dependency);

			void addSubPass(std::vector<GraphicPipeline*> p, uint32_t AttachementFlag = 0, std::vector<uint32_t> input_number = {});

			void compile();

			void draw(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, vec2i viewportMin, vec2i viewportMax, std::vector<VkClearValue> const & clear_values = {{ 1.0f, 0 }});

			VkRenderPass& getHandle();

		private : 
			VkDestroyer(VkRenderPass)															m_renderPass;
			VkFormat																							m_imageFormat;
			VkFormat																							m_depthFormat;
			std::vector<LavaCake::RenderPass::SubpassParameters>	m_subpassParameters;
			std::vector < std::vector<GraphicPipeline*>	>					m_subpass;
			std::vector<VkAttachmentReference>										m_depthAttachments;
			std::vector<VkAttachmentDescription>									m_attachmentDescriptions;
			std::vector<VkSubpassDependency>											m_dependencies;
		};
	}
}