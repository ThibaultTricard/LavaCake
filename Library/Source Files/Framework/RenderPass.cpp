#include "RenderPass.h"
namespace LavaCake {
	namespace Framework {

		void RenderPass::SpecifySubpassDescriptions(std::vector<SubpassParameters> const& subpass_parameters,
			std::vector<VkSubpassDescription>& subpass_descriptions) {
			subpass_descriptions.clear();

			for (auto& subpass_description : subpass_parameters) {
				subpass_descriptions.push_back({
					0,                                                                      // VkSubpassDescriptionFlags        flags
					subpass_description.PipelineType,                                       // VkPipelineBindPoint              pipelineBindPoint
					static_cast<uint32_t>(subpass_description.InputAttachments.size()),     // uint32_t                         inputAttachmentCount
					subpass_description.InputAttachments.data(),                            // const VkAttachmentReference    * pInputAttachments
					static_cast<uint32_t>(subpass_description.ColorAttachments.size()),     // uint32_t                         colorAttachmentCount
					subpass_description.ColorAttachments.data(),                            // const VkAttachmentReference    * pColorAttachments
					subpass_description.ResolveAttachments.data(),                          // const VkAttachmentReference    * pResolveAttachments
					subpass_description.DepthStencilAttachment,                             // const VkAttachmentReference    * pDepthStencilAttachment
					static_cast<uint32_t>(subpass_description.PreserveAttachments.size()),  // uint32_t                         preserveAttachmentCount
					subpass_description.PreserveAttachments.data()                          // const uint32_t                 * pPreserveAttachments
					});
			}
		}

		bool RenderPass::CreateRenderPass(VkDevice                                     logical_device,
			std::vector<VkAttachmentDescription> const& attachments_descriptions,
			std::vector<SubpassParameters> const& subpass_parameters,
			std::vector<VkSubpassDependency> const& subpass_dependencies,
			VkRenderPass& render_pass) {

			std::vector<VkSubpassDescription> subpass_descriptions;
			SpecifySubpassDescriptions(subpass_parameters, subpass_descriptions);

			VkRenderPassCreateInfo render_pass_create_info = {
				VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,                // VkStructureType                    sType
				nullptr,                                                  // const void                       * pNext
				0,                                                        // VkRenderPassCreateFlags            flags
				static_cast<uint32_t>(attachments_descriptions.size()),   // uint32_t                           attachmentCount
				attachments_descriptions.data(),                          // const VkAttachmentDescription    * pAttachments
				static_cast<uint32_t>(subpass_descriptions.size()),       // uint32_t                           subpassCount
				subpass_descriptions.data(),                              // const VkSubpassDescription       * pSubpasses
				static_cast<uint32_t>(subpass_dependencies.size()),       // uint32_t                           dependencyCount
				subpass_dependencies.data()                               // const VkSubpassDependency        * pDependencies
			};

			VkResult result = vkCreateRenderPass(logical_device, &render_pass_create_info, nullptr, &render_pass);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a render pass." << std::endl;
				return false;
			}
			return true;
		}

		void RenderPass::DestroyRenderPass(VkDevice       logical_device,
			VkRenderPass& render_pass) {
			if (VK_NULL_HANDLE != render_pass) {
				vkDestroyRenderPass(logical_device, render_pass, nullptr);
				render_pass = VK_NULL_HANDLE;
			}
		}


		RenderPass::RenderPass() {
			SwapChain* s = SwapChain::getSwapChain();
			m_imageFormat = s->imageFormat();
			m_depthFormat = s->depthFormat();
		}

		RenderPass::RenderPass( VkFormat ImageFormat, VkFormat DepthFormat) {
			m_imageFormat = ImageFormat;
			m_depthFormat = DepthFormat;
		}


		void RenderPass::addAttatchments(uint32_t AttachementFlag, std::vector<uint32_t> input_number) {
			VkAttachmentReference* depth = new VkAttachmentReference();
			uint32_t imageAttachementindex;
			bool drawOnScreen = false;
			if (AttachementFlag & SHOW_ON_SCREEN)
				drawOnScreen = true;

			SubpassParameters params = {};
			
			if (AttachementFlag & USE_COLOR) {
				imageAttachementindex = static_cast<uint32_t>(m_attachmentDescriptions.size());
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
				if (drawOnScreen) {
					m_khr_attachement = static_cast<int>(m_attachmentype.size());
				}
				m_attachmentype.push_back(RENDERPASS_COLOR_ATTACHMENT);
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
				m_attachmentype.push_back(RENDERPASS_DEPTH_ATTACHMENT);
				m_depthAttachments.push_back({
						uint32_t(m_attachmentDescriptions.size() - 1),														// uint32_t                             attachment
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL								// VkImageLayout                        layout
					});


				params.DepthStencilAttachment = &m_depthAttachments[m_depthAttachments.size() - 1];

			}
			m_subpassAttachements.push_back(input_number);
			if (AttachementFlag & ADD_INPUT) {
				std::vector<VkAttachmentReference>   inputAttachments = {};
				for (size_t i = 0; i < input_number.size(); i++) {
					inputAttachments.push_back({ input_number[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
					for (size_t j = 0; j < m_attachmentype.size(); j++) {
						if (m_attachmentype[j] == RENDERPASS_COLOR_ATTACHMENT) {
							if (j == input_number[i]) {
								m_attachmentype[j] = RENDERPASS_INPUT_ATTACHMENT;
							}
						}
					}
				}
				params.InputAttachments = inputAttachments;
				
				
			}

			m_subpassParameters.push_back(params);
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

		void RenderPass::addSubPass(std::vector<GraphicPipeline*> p, uint32_t AttachementFlag, std::vector<uint32_t> input_number) {
			for (size_t i = 0; i < p.size(); i++) {
				p[i]->setSubpassNumber(static_cast<uint32_t>(m_subpass.size()));
			}
			m_subpass.push_back(p);
			
			addAttatchments(AttachementFlag, input_number);
		}

		void RenderPass::compile() {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logicalDevice = d->getLogicalDevice();

			InitVkDestroyer(logicalDevice, m_renderPass);
			if (!CreateRenderPass(logicalDevice, m_attachmentDescriptions, m_subpassParameters, m_dependencies, *m_renderPass)) {
				ErrorCheck::setError("Can't compile RenderPass");
			}

			for (uint32_t i = 0; i < m_subpass.size(); i++) {
				for (uint32_t j = 0; j < m_subpass[i].size(); j++) {
					m_subpass[i][j]->compile(*m_renderPass);
				}
			}

			std::vector<Attachment*> tempInputAttachements = std::vector<Attachment*>(m_attachmentype.size());

			for (size_t i = 0; i < m_subpassAttachements.size(); i++) {
				tempInputAttachements[i] = nullptr;
				std::vector<Attachment*> tia;
				for (size_t k = 0; k < m_subpass[i].size(); k++) {
					std::vector<attachment> attachment = m_subpass[i][k]->getAttachments();
					for (size_t l = 0; l < attachment.size(); l++) {
						bool insert = true;
						for (int m = 0; m < tia.size(); m++) {
							if (attachment[l].a == tia[m]) {
								insert = false;
							}
						}
						if (insert) {
							tia.push_back(attachment[l].a);
						}
					}
				}
				for (size_t j = 0; j < tia.size(); j++) {
					tempInputAttachements[m_subpassAttachements[i][j]] = tia[j];
				}
			}

			for (size_t i = 0; i < m_subpassAttachements.size(); i++) {
				if (tempInputAttachements[i] != nullptr) {
					m_inputAttachements.push_back(tempInputAttachements[i]);
				}
			}

		}

		void RenderPass::reloadShaders() {

			for (uint32_t i = 0; i < m_subpass.size(); i++) {
				for (uint32_t j = 0; j < m_subpass[i].size(); j++) {
					m_subpass[i][j]->reloadShaders();
				}
			}

			compile();

		}

		void RenderPass::draw(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, vec2u viewportMin, vec2u viewportMax, std::vector<VkClearValue> const & clear_values) {

			VkRenderPassBeginInfo renderPassBeginInfo = {
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,																																														 // VkStructureType        sType
				nullptr,																																																														 // const void           * pNext
				* m_renderPass,																																																											 // VkRenderPass           renderPass
				frameBuffer,																																																												 // VkFramebuffer          framebuffer
				{ { 0, 0 },{uint32_t(viewportMax[0] - viewportMin[0]),uint32_t(viewportMax[1] - viewportMin[1])} },                                  // VkRect2D               renderArea
				static_cast<uint32_t>(clear_values.size()),																																													 // uint32_t               clearValueCount
				clear_values.data()																																																									 // const VkClearValue   * pClearValues
			};

			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			for (uint32_t i = 0; i < m_subpass.size(); i++) {

				if (i > 0) {
					vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
				}
				
				for (uint32_t j = 0; j < m_subpass[i].size(); j++) {
					m_subpass[i][j]->draw(commandBuffer);
				}
			}

			vkCmdEndRenderPass(commandBuffer);
		}


		VkRenderPass& RenderPass::getHandle() {
			return *m_renderPass;
		}

		void RenderPass::prepareOutputFrameBuffer(FrameBuffer& frameBuffer) {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();
			

			InitVkDestroyer(logical, frameBuffer.m_sampler);
			InitVkDestroyer(logical, frameBuffer.m_imageMemory);
		
			if (!LavaCake::Buffer::CreateSampler(logical, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 
				0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, false, *frameBuffer.m_sampler)) {
				ErrorCheck::setError("Can't create an image sampler for this FrameBuffer");
			}

			bool linear_filtering = true;

			VkImageUsageFlagBits usage;
			VkImageAspectFlagBits aspect;
			VkImageLayout layout;
			VkFormat format;
			
			frameBuffer.m_images = std::vector<VkImage>(m_attachmentype.size());
			frameBuffer.m_imageViews = std::vector<VkImageView>(m_attachmentype.size());

			int attachementIndex = 0;

			for (int i = 0; i < m_attachmentype.size(); i ++) {
				

				if (m_attachmentype[i] == RENDERPASS_COLOR_ATTACHMENT) {
					format = m_imageFormat;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) | static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLED_BIT));
					aspect = VK_IMAGE_ASPECT_COLOR_BIT;
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				else if (m_attachmentype[i] == RENDERPASS_INPUT_ATTACHMENT) {
					format = m_imageFormat;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) | static_cast<uint32_t>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
					aspect = VK_IMAGE_ASPECT_COLOR_BIT;
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


					frameBuffer.m_images[i] = m_inputAttachements[attachementIndex]->getImage();
					frameBuffer.m_imageViews[i] = m_inputAttachements[attachementIndex]->getImageView();

					attachementIndex++;
					continue;
				}
				else if (m_attachmentype[i] == RENDERPASS_DEPTH_ATTACHMENT) {
					format = m_depthFormat;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) | static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLED_BIT));
					aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
					layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;


				}
				else {
					format = VK_FORMAT_UNDEFINED;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM) | static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLED_BIT));
					aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
					layout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
				frameBuffer.m_layouts.push_back(layout);

				if (i == m_khr_attachement) {
					continue;
				}

				frameBuffer.m_images[i] = VkImage();
				frameBuffer.m_imageViews[i] =  VkImageView();


				if (!Image::CreateSampledImage(physical, logical, VK_IMAGE_TYPE_2D, format,{ (uint32_t)frameBuffer.m_width, (uint32_t)frameBuffer.m_height, 1 }, 1, 1, usage, false, VK_IMAGE_VIEW_TYPE_2D, aspect, linear_filtering, frameBuffer.m_images[i], *frameBuffer.m_imageMemory, frameBuffer.m_imageViews[i])) {
					ErrorCheck::setError("Can't create an image sampler for this FrameBuffer");
				}
				
			}
			if (m_khr_attachement == -1) {
				if (!LavaCake::Buffer::CreateFramebuffer(logical, *m_renderPass, frameBuffer.m_imageViews, frameBuffer.m_width, frameBuffer.m_height, 1, *frameBuffer.m_frameBuffer)) {
					ErrorCheck::setError("Can't create this FrameBuffer");
				}
			}
		}

		void RenderPass::setSwapChainImage(FrameBuffer& frameBuffer, SwapChainImage& image) {
			if (m_khr_attachement != -1) {
				
				
				Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				LavaCake::Buffer::DestroyFramebuffer(logical, *frameBuffer.m_frameBuffer);
				frameBuffer.m_images[m_khr_attachement] = image.getImage();
				frameBuffer.m_imageViews[m_khr_attachement] = image.getView();
				frameBuffer.m_swapChainImageIndex = m_khr_attachement;
				if (!LavaCake::Buffer::CreateFramebuffer(logical, *m_renderPass, frameBuffer.m_imageViews, frameBuffer.m_width, frameBuffer.m_height, 1, *frameBuffer.m_frameBuffer)) {
					ErrorCheck::setError("Can't create this FrameBuffer");
				}
			}
		}
	}
}