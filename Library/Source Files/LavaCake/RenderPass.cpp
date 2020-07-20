#include "RenderPass.h"
namespace LavaCake {
	namespace Framework {
		RenderPass::RenderPass(attachementType type, boolean drawOnScreen) {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			m_imageFormat = d->getSwapChain().imageFormat();
			m_depthFormat = d->getSwapChain().depthFormat();

			setupAttatchments(type, drawOnScreen);


		}

		RenderPass::RenderPass(attachementType type, boolean drawOnScreen, VkFormat ImageFormat, VkFormat DepthFormat) {
			m_imageFormat = ImageFormat;
			m_depthFormat = DepthFormat;
			setupAttatchments(type, drawOnScreen);
		}


		void RenderPass::setupAttatchments(attachementType type, boolean drawOnScreen) {
			VkAttachmentReference* depth = new VkAttachmentReference();
			uint32_t imageAttachementindex = uint32_t(m_attachmentDescriptions.size());
			switch (type) {
			case ImageOnly:
				m_attachmentDescriptions.push_back(
					{
						0,																																																					// VkAttachmentDescriptionFlags     flags
						m_imageFormat,																																																// VkFormat                         format
						VK_SAMPLE_COUNT_1_BIT,																																											// VkSampleCountFlagBits            samples
						VK_ATTACHMENT_LOAD_OP_CLEAR,																																								// VkAttachmentLoadOp               loadOp
						VK_ATTACHMENT_STORE_OP_STORE,																																								// VkAttachmentStoreOp              storeOp
						VK_ATTACHMENT_LOAD_OP_DONT_CARE,																																						// VkAttachmentLoadOp               stencilLoadOp
						VK_ATTACHMENT_STORE_OP_DONT_CARE,																																						// VkAttachmentStoreOp              stencilStoreOp
						VK_IMAGE_LAYOUT_UNDEFINED,																																									// VkImageLayout                    initialLayout
						drawOnScreen ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                   // VkImageLayout                    finalLayout
					});


				m_subpassParameters.push_back(
					{
						VK_PIPELINE_BIND_POINT_GRAPHICS,              // VkPipelineBindPoint                  PipelineType
						{},                                           // std::vector<VkAttachmentReference>   InputAttachments
						{                                             // std::vector<VkAttachmentReference>   ColorAttachments
							{
								imageAttachementindex,                    // uint32_t                             attachment
								VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout                        layout
							}
						},
						{},                                           // std::vector<VkAttachmentReference>   ResolveAttachments
						{},																				// VkAttachmentReference const        * DepthStencilAttachment
						{}                                            // std::vector<uint32_t>                PreserveAttachments
					}
				);
				break;

			case ImageAndDepth:

				m_attachmentDescriptions.push_back(
					{
						0,																																																					// VkAttachmentDescriptionFlags     flags
						m_imageFormat,																																																// VkFormat                         format
						VK_SAMPLE_COUNT_1_BIT,																																											// VkSampleCountFlagBits            samples
						VK_ATTACHMENT_LOAD_OP_CLEAR,																																								// VkAttachmentLoadOp               loadOp
						VK_ATTACHMENT_STORE_OP_STORE,																																								// VkAttachmentStoreOp              storeOp
						VK_ATTACHMENT_LOAD_OP_DONT_CARE,																																						// VkAttachmentLoadOp               stencilLoadOp
						VK_ATTACHMENT_STORE_OP_DONT_CARE,																																						// VkAttachmentStoreOp              stencilStoreOp
						VK_IMAGE_LAYOUT_UNDEFINED,																																									// VkImageLayout                    initialLayout
						drawOnScreen ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                   // VkImageLayout                    finalLayout
					});

				m_attachmentDescriptions.push_back({
					0,                                                // VkAttachmentDescriptionFlags     flags
					m_depthFormat,                                    // VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  // VkImageLayout                    finalLayout
					});

				m_depthAttachments.push_back({
						uint32_t(m_attachmentDescriptions.size() - 1),                  // uint32_t                             attachment
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  // VkImageLayout                        layout
					});

				depth = &m_depthAttachments[m_depthAttachments.size() - 1];


				m_subpassParameters.push_back(
					{
						VK_PIPELINE_BIND_POINT_GRAPHICS,              // VkPipelineBindPoint                  PipelineType
						{},                                           // std::vector<VkAttachmentReference>   InputAttachments
						{                                             // std::vector<VkAttachmentReference>   ColorAttachments
							{
								imageAttachementindex,                    // uint32_t                             attachment
								VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout                        layout
							}
						},
						{},                                           // std::vector<VkAttachmentReference>   ResolveAttachments
						depth,																				// VkAttachmentReference const        * DepthStencilAttachment
						{}                                            // std::vector<uint32_t>                PreserveAttachments
					}
				);
				break;

			case DepthOnly:
				m_attachmentDescriptions.push_back({
					0,                                                // VkAttachmentDescriptionFlags     flags
					m_depthFormat,                                    // VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
					VK_ATTACHMENT_STORE_OP_STORE,											// VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL		// VkImageLayout                    finalLayout
					});

				m_depthAttachments.push_back({
						0,																															// uint32_t                             attachment
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL								// VkImageLayout                        layout
					});

				m_subpassParameters.push_back(
					{
						VK_PIPELINE_BIND_POINT_GRAPHICS,              // VkPipelineBindPoint                  PipelineType
						{},                                           // std::vector<VkAttachmentReference>   InputAttachments
						{},
						{},                                           // std::vector<VkAttachmentReference>   ResolveAttachments
						&m_depthAttachments[m_depthAttachments.size() - 1] ,																			// VkAttachmentReference const        * DepthStencilAttachment
						{}                                            // std::vector<uint32_t>                PreserveAttachments
					}
				);
				break;

			}
		}

		void RenderPass::addDependencies(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcPipe, VkPipelineStageFlags dstPipe, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkDependencyFlags dependency) {
			m_dependencies.push_back({
					srcSubpass,																			// uint32_t                   srcSubpass
					dstSubpass,                                     // uint32_t                   dstSubpass
					srcPipe,														            // VkPipelineStageFlags       srcStageMask
					dstPipe,																			  // VkPipelineStageFlags       dstStageMask
					srcAccess,																			// VkAccessFlags              srcAccessMask
					dstAccess,																			// VkAccessFlags              dstAccessMask
					dependency																			// VkDependencyFlags          dependencyFlags
				});
		}

		void RenderPass::addSubPass(GraphicPipeline* p) {
			m_pipelines.push_back(p);
		}

		void RenderPass::compile() {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logicalDevice = d->getLogicalDevice();

			InitVkDestroyer(logicalDevice, m_renderPass);
			if (!LavaCake::RenderPass::CreateRenderPass(logicalDevice, m_attachmentDescriptions, m_subpassParameters, m_dependencies, *m_renderPass)) {
				ErrorCheck::setError(5, "Cannot compile RenderPass");
			}

			for (int i = 0; i < m_pipelines.size(); i++) {
				m_pipelines[i]->compile(*m_renderPass);
			}
		}

		void RenderPass::draw(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, vec2i viewportMin, vec2i viewportMax, vec4f clearColor) {
			LavaCake::RenderPass::BeginRenderPass(commandBuffer, *m_renderPass, frameBuffer, { { 0, 0 },
				{uint32_t(viewportMax[0] - viewportMin[0]),uint32_t(viewportMax[1] - viewportMin[1])} },
				{ {clearColor[0],clearColor[1],clearColor[2],clearColor[3]}, { 1.0f, 0 } }, VK_SUBPASS_CONTENTS_INLINE);
			for (int i = 0; i < m_pipelines.size(); i++) {
				m_pipelines[i]->draw(commandBuffer);
			}


			LavaCake::RenderPass::EndRenderPass(commandBuffer);
		}

		void RenderPass::draw(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, vec2i viewportMin, vec2i viewportMax) {
			LavaCake::RenderPass::BeginRenderPass(commandBuffer, *m_renderPass, frameBuffer, { { 0, 0, },
				{uint32_t(viewportMax[0] - viewportMin[0]),uint32_t(viewportMax[1] - viewportMin[1])} },
				{ { 1.0f, 0 } }, VK_SUBPASS_CONTENTS_INLINE);
			for (int i = 0; i < m_pipelines.size(); i++) {
				m_pipelines[i]->draw(commandBuffer);
			}


			LavaCake::RenderPass::EndRenderPass(commandBuffer);
		}

		VkRenderPass& RenderPass::getHandle() {
			return *m_renderPass;
		}
	}
}