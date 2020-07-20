#include "RenderPass.h"
namespace LavaCake {
	namespace Framework {
		RenderPass::RenderPass(uint32_t AttachementFlag) {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			m_imageFormat = d->getSwapChain().imageFormat();
			m_depthFormat = d->getSwapChain().depthFormat();

			setupAttatchments(AttachementFlag);


		}

		RenderPass::RenderPass(uint32_t AttachementFlag, VkFormat ImageFormat, VkFormat DepthFormat) {
			m_imageFormat = ImageFormat;
			m_depthFormat = DepthFormat;
			setupAttatchments(AttachementFlag);
		}


		void RenderPass::setupAttatchments(uint32_t AttachementFlag) {
			VkAttachmentReference* depth = new VkAttachmentReference();
			uint32_t imageAttachementindex;
			bool drawOnScreen = false;
			if (AttachementFlag & SHOW_ON_SCREEN)
				drawOnScreen = true;

			LavaCake::RenderPass::SubpassParameters params = {};
			
			if (AttachementFlag & USE_COLOR) {
				imageAttachementindex = m_attachmentDescriptions.size();
				m_attachmentDescriptions.push_back(
					{
						0,																																																					// VkAttachmentDescriptionFlags     flags
						m_imageFormat,																																															// VkFormat                         format
						VK_SAMPLE_COUNT_1_BIT,																																											// VkSampleCountFlagBits            samples
						VK_ATTACHMENT_LOAD_OP_CLEAR,																																								// VkAttachmentLoadOp               loadOp
						AttachementFlag & OP_STORE_COLOR ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,					// VkAttachmentStoreOp              storeOp
						VK_ATTACHMENT_LOAD_OP_DONT_CARE,																																						// VkAttachmentLoadOp               stencilLoadOp
						VK_ATTACHMENT_STORE_OP_DONT_CARE,																																						// VkAttachmentStoreOp              stencilStoreOp
						VK_IMAGE_LAYOUT_UNDEFINED,																																									// VkImageLayout                    initialLayout
						drawOnScreen ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                   // VkImageLayout                    finalLayout
					});

				params.ColorAttachments = 
				{ 
					{
						imageAttachementindex,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					} 
				};
			}

			if (AttachementFlag & USE_DEPTH) {
				m_attachmentDescriptions.push_back({
					0,																																																									// VkAttachmentDescriptionFlags     flags
					m_depthFormat,																																																			// VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,																																															// VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,																																												// VkAttachmentLoadOp               loadOp
					AttachementFlag & OP_STORE_DEPTH ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE ,								// VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,																																										// VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,																																										// VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,																																													// VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL																																			// VkImageLayout                    finalLayout
					});

				m_depthAttachments.push_back({
						m_attachmentDescriptions.size() - 1,														// uint32_t                             attachment
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL								// VkImageLayout                        layout
					});


				params.DepthStencilAttachment = &m_depthAttachments[m_depthAttachments.size() - 1];

				m_subpassParameters.push_back( params );

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
				ErrorCheck::setError("Can't compile RenderPass");
			}

			for (int i = 0; i < m_pipelines.size(); i++) {
				m_pipelines[i]->compile(*m_renderPass);
			}
		}

		void RenderPass::draw(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, vec2i viewportMin, vec2i viewportMax, std::vector<VkClearValue> const & clear_values) {
			LavaCake::RenderPass::BeginRenderPass(commandBuffer, *m_renderPass, frameBuffer, { { 0, 0 },
				{uint32_t(viewportMax[0] - viewportMin[0]),uint32_t(viewportMax[1] - viewportMin[1])} },
				clear_values, VK_SUBPASS_CONTENTS_INLINE);
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