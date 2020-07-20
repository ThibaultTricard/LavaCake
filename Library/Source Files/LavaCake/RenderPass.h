#pragma once
#include "AllHeaders.h"
#include "LavaCake/GraphicPipeline.h"

namespace LavaCake {
	namespace Framework {
		
		enum attachementType {
			ImageOnly, DepthOnly, ImageAndDepth
		};


		class RenderPass {
		public :

			RenderPass(attachementType type, boolean drawOnScreen);

			RenderPass(attachementType type, boolean drawOnScreen, VkFormat ImageFormat, VkFormat DepthFormat);


			void setupAttatchments(attachementType type, boolean drawOnScreen);

			void addDependencies(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcPipe, VkPipelineStageFlags dstPipe, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkDependencyFlags dependency);

			void addSubPass(GraphicPipeline* p);

			void compile();

			void draw(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, vec2i viewportMin, vec2i viewportMax, vec4f clearColor);

			void draw(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, vec2i viewportMin, vec2i viewportMax);

			VkRenderPass& getHandle();

		private : 
			VkDestroyer(VkRenderPass)															m_renderPass;
			VkFormat																							m_imageFormat;
			VkFormat																							m_depthFormat;
			std::vector<LavaCake::RenderPass::SubpassParameters>	m_subpassParameters;
			std::vector<GraphicPipeline*>													m_pipelines;
			std::vector<VkAttachmentReference>										m_depthAttachments;
			std::vector<VkAttachmentDescription>									m_attachmentDescriptions;
			std::vector<VkSubpassDependency>											m_dependencies;
		};
	}
}