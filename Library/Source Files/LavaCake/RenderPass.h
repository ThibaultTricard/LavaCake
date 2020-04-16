#pragma once
#include "AllHeaders.h"
#include "Pipeline.h"

namespace LavaCake {
	namespace Framework {

		class RenderPass {
		public :

			RenderPass() {
				LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
				m_imageFormat = d->getSwapChain().imageFormat();
				m_depthFormat = d->getSwapChain().depthFormat();
			}

			RenderPass(VkFormat ImageFormat, VkFormat DepthFormat) {
				m_imageFormat = ImageFormat;
				m_depthFormat = DepthFormat;
			}

			void addDependencies(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcPipe, VkPipelineStageFlags dstPipe, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkDependencyFlags dependency){
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

			int addSubPass(GraphicPipeline* p, boolean drawOnScreen, boolean addDepthAttachment) {
				m_pipelines.push_back(p);
				VkAttachmentReference* depth = new VkAttachmentReference();
				uint32_t imageAttachementindex = uint32_t(m_attachmentDescriptions.size());
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
				if (addDepthAttachment) {
					m_attachmentDescriptions.push_back({
						0,                                                // VkAttachmentDescriptionFlags     flags
						m_depthFormat,                                      // VkFormat                         format
						VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
						VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
						VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              storeOp
						VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
						VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
						VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  // VkImageLayout                    finalLayout
					});

					m_depthAttachments.push_back( {
							uint32_t(m_attachmentDescriptions.size()-1),                  // uint32_t                             attachment
							VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  // VkImageLayout                        layout
					});

					depth = &m_depthAttachments[m_depthAttachments.size() - 1];
				}

				
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
				return int(m_subpassParameters.size() - 1);
		}

			void compile() {
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

			void draw(const Buffer::FrameResources& ressources, vec2i viewportMin, vec2i viewportMax) {
				LavaCake::RenderPass::BeginRenderPass(ressources.CommandBuffer, *m_renderPass, *ressources.Framebuffer, { { 0, 0 },
					{uint32_t(viewportMax[0] - viewportMin[0]),uint32_t(viewportMax[1] - viewportMin[1])}},
					{ { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } }, VK_SUBPASS_CONTENTS_INLINE);
				for (int i = 0; i < m_pipelines.size(); i++) {
					m_pipelines[i]->draw(ressources);
				}
				

				LavaCake::RenderPass::EndRenderPass(ressources.CommandBuffer);
			}

			VkRenderPass& getHandle() {
				return *m_renderPass;
			}

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