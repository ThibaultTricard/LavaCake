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
			
			RenderPassAttachmentType it = toAttachmentType(ImageFormat);
			if (it & RenderPassAttachmentType::RENDERPASS_COLOR_ATTACHMENT) {
				m_imageFormat = ImageFormat;
			}
			else {
				ErrorCheck::setError("Image format not recognized", 1);
			}
			RenderPassAttachmentType dt = toAttachmentType(DepthFormat);
			if (dt & RenderPassAttachmentType::RENDERPASS_DEPTH_ATTACHMENT || dt & RenderPassAttachmentType::RENDERPASS_STENCIL_ATTACHMENT) {
				m_depthFormat = DepthFormat;
			}
			else {
				ErrorCheck::setError("Depth format not recognized", 1);
			}
		}


		void RenderPass::addAttatchments(SubpassAttachment AttachementDescription, std::vector<uint32_t> input_number) {
      uint32_t imageAttachementindex;
			bool drawOnScreen = false;
			if (AttachementDescription.showOnScreen)
				drawOnScreen = true;

			SubpassParameters params = {};
			
			if (AttachementDescription.nbColor > 0) {
				params.ColorAttachments = {};
				for (uint16_t c = 0; c < AttachementDescription.nbColor; c++) {
					imageAttachementindex = static_cast<uint32_t>(m_attachmentDescriptions.size());
					m_attachmentDescriptions.push_back(
						{
							0,																																																					// VkAttachmentDescriptionFlags     flags
							m_imageFormat,																																															// VkFormat                         format
							VK_SAMPLE_COUNT_1_BIT,																																											// VkSampleCountFlagBits            samples
							VK_ATTACHMENT_LOAD_OP_CLEAR,																																								// VkAttachmentLoadOp               loadOp
							AttachementDescription.storeColor ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,					// VkAttachmentStoreOp              storeOp
							VK_ATTACHMENT_LOAD_OP_DONT_CARE,																																						// VkAttachmentLoadOp               stencilLoadOp
							VK_ATTACHMENT_STORE_OP_DONT_CARE,																																						// VkAttachmentStoreOp              stencilStoreOp
							VK_IMAGE_LAYOUT_UNDEFINED,																																									// VkImageLayout                    initialLayout
							(c == AttachementDescription.nbColor -1 -AttachementDescription.showOnScreenIndex && drawOnScreen) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                   // VkImageLayout                    finalLayout
						});
				
					m_attachmentype.push_back(RENDERPASS_COLOR_ATTACHMENT);
					params.ColorAttachments.push_back(
						{
							imageAttachementindex,
							VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						}
					);
				}

				if (drawOnScreen) {
					m_khr_attachement = static_cast<int>(m_attachmentype.size()) -1 -AttachementDescription.showOnScreenIndex;
				}
				

				/*params.ColorAttachments = 
				{ 
					{
						imageAttachementindex,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					} 
				};*/
			}

			if (AttachementDescription.useDepth) {
				m_attachmentDescriptions.push_back({
					0,																																																									// VkAttachmentDescriptionFlags     flags
					m_depthFormat,																																																			// VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,																																															// VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,																																												// VkAttachmentLoadOp               loadOp
					AttachementDescription.storeDepth ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE ,								// VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,																																										// VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,																																										// VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,																																													// VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL																																			// VkImageLayout                    finalLayout
					});
				RenderPassAttachmentType dt = toAttachmentType(m_depthFormat);
				m_attachmentype.push_back(dt);
				m_depthAttachments.push_back({
						uint32_t(m_attachmentDescriptions.size() - 1),														// uint32_t                             attachment
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL								// VkImageLayout                        layout
					});


				params.DepthStencilAttachment = &m_depthAttachments[m_depthAttachments.size() - 1];

			}
			m_subpassAttachements.push_back(input_number);
			if (AttachementDescription.addInput) {
				std::vector<VkAttachmentReference>   inputAttachments = {};
				for (size_t i = 0; i < input_number.size(); i++) {
					inputAttachments.push_back({ input_number[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
					for (size_t j = 0; j < m_attachmentype.size(); j++) {
						if (m_attachmentype[j] & RENDERPASS_COLOR_ATTACHMENT) {
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

		void RenderPass::addSubPass(std::vector<GraphicPipeline*> p, SubpassAttachment AttachementDescription, std::vector<uint32_t> input_number) {
			for (size_t i = 0; i < p.size(); i++) {
				p[i]->setSubpassNumber(static_cast<uint32_t>(m_subpass.size()));
			}
			m_subpass.push_back(p);
			
			addAttatchments(AttachementDescription, input_number);
		}

		void RenderPass::compile() {
			Device* d = Device::getDevice();
			VkDevice logicalDevice = d->getLogicalDevice();

			if (!CreateRenderPass(logicalDevice, m_attachmentDescriptions, m_subpassParameters, m_dependencies, m_renderPass)) {
				ErrorCheck::setError("Can't compile RenderPass");
			}

			for (uint32_t i = 0; i < m_subpass.size(); i++) {
				for (uint32_t j = 0; j < m_subpass[i].size(); j++) {
					m_subpass[i][j]->compile(m_renderPass, (uint16_t)m_subpassParameters[i].ColorAttachments.size());
				}
			}

			std::vector<Image*> tempInputAttachements = std::vector<Image*>(m_attachmentype.size());

			for (size_t i = 0; i < m_subpassAttachements.size(); i++) {
				tempInputAttachements[i] = nullptr;
				std::vector<Image*> tia;
				for (size_t k = 0; k < m_subpass[i].size(); k++) {
					std::vector<attachment> attachment = m_subpass[i][k]->getAttachments();
					for (size_t l = 0; l < attachment.size(); l++) {
						bool insert = true;
						for (int m = 0; m < tia.size(); m++) {
							if (attachment[l].i == tia[m]) {
								insert = false;
							}
						}
						if (insert) {
							tia.push_back(attachment[l].i);
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

		void RenderPass::draw(CommandBuffer& commandBuffer, FrameBuffer& frameBuffer, vec2u viewportMin, vec2u viewportMax, std::vector<VkClearValue> const & clear_values) {

			VkRenderPassBeginInfo renderPassBeginInfo = {
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,																																														 // VkStructureType        sType
				nullptr,																																																														 // const void           * pNext
        m_renderPass,																																																											 // VkRenderPass           renderPass
				frameBuffer.getHandle(),																																																						 // VkFramebuffer          framebuffer
				{ { 0, 0 },{uint32_t(viewportMax[0] - viewportMin[0]),uint32_t(viewportMax[1] - viewportMin[1])} },                                  // VkRect2D               renderArea
				static_cast<uint32_t>(clear_values.size()),																																													 // uint32_t               clearValueCount
				clear_values.data()																																																									 // const VkClearValue   * pClearValues
			};

			vkCmdBeginRenderPass(commandBuffer.getHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			for (uint32_t i = 0; i < m_subpass.size(); i++) {

				if (i > 0) {
					vkCmdNextSubpass(commandBuffer.getHandle(), VK_SUBPASS_CONTENTS_INLINE);
				}
				
				for (uint32_t j = 0; j < m_subpass[i].size(); j++) {
					m_subpass[i][j]->draw(commandBuffer);
				}
			}

			vkCmdEndRenderPass(commandBuffer.getHandle());
		}


		VkRenderPass& RenderPass::getHandle() {
			return m_renderPass;
		}

		void RenderPass::prepareOutputFrameBuffer(Queue* queue, CommandBuffer& commandBuffer, FrameBuffer& frameBuffer) {
			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			
			VkSamplerCreateInfo sampler_create_info = {
				VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,    // VkStructureType          sType
				nullptr,                                  // const void             * pNext
				0,                                        // VkSamplerCreateFlags     flags
				VK_FILTER_LINEAR,                         // VkFilter                 magFilter
				VK_FILTER_LINEAR,                         // VkFilter                 minFilter
				VK_SAMPLER_MIPMAP_MODE_NEAREST,           // VkSamplerMipmapMode      mipmapMode
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // VkSamplerAddressMode     addressModeU
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // VkSamplerAddressMode     addressModeV
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,    // VkSamplerAddressMode     addressModeW
				0.0f,																			// float                    mipLodBias
				false,																		// VkBool32                 anisotropyEnable
				1.0f,																			// float                    maxAnisotropy
				false,																		// VkBool32                 compareEnable
				VK_COMPARE_OP_ALWAYS,                     // VkCompareOp              compareOp
				0.0f,																			// float                    minLod
				1.0f,																			// float                    maxLod
				VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,       // VkBorderColor            borderColor
				false																			// VkBool32                 unnormalizedCoordinates
			};

			VkResult result = vkCreateSampler(logical, &sampler_create_info, nullptr, &frameBuffer.m_sampler);
			if (VK_SUCCESS != result) {
				ErrorCheck::setError("Could not create sampler for this FrameBuffer.");
			}

			bool linear_filtering = true;

			VkImageUsageFlagBits usage;
			VkImageAspectFlagBits aspect;
			VkImageLayout layout;
			VkFormat format;
			
			frameBuffer.m_images = std::vector<Image*>(m_attachmentype.size());

			int attachementIndex = 0;


			commandBuffer.beginRecord();
			for (int i = 0; i < m_attachmentype.size(); i ++) {
				

				if (m_attachmentype[i] & RENDERPASS_COLOR_ATTACHMENT) {
					format = m_imageFormat;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) | static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLED_BIT));
					aspect = VK_IMAGE_ASPECT_COLOR_BIT;
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				else if (m_attachmentype[i] == RENDERPASS_INPUT_ATTACHMENT) {
          // comment : because of  continue, the following 4 values are never read (else case next turn overwrite them before use)
					format = m_imageFormat;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) | static_cast<uint32_t>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
					aspect = VK_IMAGE_ASPECT_COLOR_BIT;
					layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


					frameBuffer.m_images[i] = m_inputAttachements[attachementIndex];
					//frameBuffer.m_imageViews[i] = m_inputAttachements[attachementIndex]->getImageView();

					attachementIndex++;
					continue;
				}
				else if (m_attachmentype[i] & RENDERPASS_DEPTH_ATTACHMENT || m_attachmentype[i] & RENDERPASS_STENCIL_ATTACHMENT) {
					format = m_depthFormat;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) | static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLED_BIT));
					
					if (m_attachmentype[i] & RENDERPASS_DEPTH_ATTACHMENT && m_attachmentype[i] & RENDERPASS_STENCIL_ATTACHMENT) {
						aspect = VkImageAspectFlagBits(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
					}
					else if(m_attachmentype[i] & RENDERPASS_DEPTH_ATTACHMENT){
						aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
					}
					else {
						aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
					}

					layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;


				}
				else {
					format = VK_FORMAT_UNDEFINED;
					usage = static_cast<VkImageUsageFlagBits>(static_cast<uint32_t>(VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM) | static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLED_BIT));
					aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
					layout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
				//frameBuffer.m_layouts.push_back(layout);

				if (i == m_khr_attachement) {
					frameBuffer.m_swapChainImageIndex = i;
					continue;
				}

        
        frameBuffer.m_images[i] = new Image((uint32_t)frameBuffer.m_width, (uint32_t)frameBuffer.m_height, 1, format, aspect, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,false);
				
				VkImageSubresourceRange subresourceRange{ (VkImageAspectFlags)aspect, 0, 1, 0, 1 };

				frameBuffer.m_images[i]->setLayout(commandBuffer, layout, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresourceRange);

			}

			commandBuffer.endRecord();
			commandBuffer.submit(queue, {}, {});
			commandBuffer.wait(UINT32_MAX);
			commandBuffer.resetFence();

			if (m_khr_attachement == -1) {
        std::vector<VkImageView> imageViews{};
        for(auto i : frameBuffer.m_images){
          imageViews.push_back(i->getImageView());
        }
        
        VkFramebufferCreateInfo framebuffer_create_info = {
          VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,                 // VkStructureType              sType
          nullptr,                                                   // const void                 * pNext
          0,                                                         // VkFramebufferCreateFlags     flags
          m_renderPass,                                             // VkRenderPass                 renderPass
          static_cast<uint32_t>(imageViews.size()),    // uint32_t                     attachmentCount
          imageViews.data(),                           // const VkImageView          * pAttachments
          frameBuffer.m_width,                                       // uint32_t                     width
          frameBuffer.m_height,                                      // uint32_t                     height
          1                                                          // uint32_t                     layers
        };
        
        VkResult result = vkCreateFramebuffer(logical, &framebuffer_create_info, nullptr, &frameBuffer.m_frameBuffer);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError( "Could not create a framebuffer.");
        }
        
			}
		}

		void RenderPass::setSwapChainImage(FrameBuffer& frameBuffer, SwapChainImage& image) {
			if (m_khr_attachement != -1) {
        std::vector<VkImageView> imageViews{};
        for(auto i : frameBuffer.m_images){
          if(i!= nullptr){
            imageViews.push_back(i->getImageView());
          }
          else{
            imageViews.push_back(VK_NULL_HANDLE);
          }
        }
				
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				if (VK_NULL_HANDLE != frameBuffer.m_frameBuffer) {
					vkDestroyFramebuffer(logical, frameBuffer.m_frameBuffer, nullptr);
					frameBuffer.m_frameBuffer = VK_NULL_HANDLE;
				}
        imageViews[m_khr_attachement] = image.getView();
        
        VkFramebufferCreateInfo framebuffer_create_info = {
          VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,                 // VkStructureType              sType
          nullptr,                                                   // const void                 * pNext
          0,                                                         // VkFramebufferCreateFlags     flags
          m_renderPass,                                             // VkRenderPass                 renderPass
          static_cast<uint32_t>(imageViews.size()),    // uint32_t                     attachmentCount
          imageViews.data(),                           // const VkImageView          * pAttachments
          frameBuffer.m_width,                                       // uint32_t                     width
          frameBuffer.m_height,                                      // uint32_t                     height
          1                                                          // uint32_t                     layers
        };
        
        VkResult result = vkCreateFramebuffer(logical, &framebuffer_create_info, nullptr, &frameBuffer.m_frameBuffer);
        if (VK_SUCCESS != result) {
          ErrorCheck::setError( "Could not create a framebuffer.");
        }
			}
		}






		RenderPassAttachmentType toAttachmentType(VkFormat format) {
			if (
				format == VK_FORMAT_R4G4_UNORM_PACK8 ||
				format == VK_FORMAT_R4G4B4A4_UNORM_PACK16 ||
				format == VK_FORMAT_B4G4R4A4_UNORM_PACK16 ||
				format == VK_FORMAT_R5G6B5_UNORM_PACK16 ||
				format == VK_FORMAT_B5G6R5_UNORM_PACK16 ||
				format == VK_FORMAT_R5G5B5A1_UNORM_PACK16 ||
				format == VK_FORMAT_B5G5R5A1_UNORM_PACK16 ||
				format == VK_FORMAT_A1R5G5B5_UNORM_PACK16 ||
				format == VK_FORMAT_R8_UNORM ||
				format == VK_FORMAT_R8_SNORM ||
				format == VK_FORMAT_R8_USCALED ||
				format == VK_FORMAT_R8_SSCALED ||
				format == VK_FORMAT_R8_UINT ||
				format == VK_FORMAT_R8_SINT ||
				format == VK_FORMAT_R8_SRGB ||
				format == VK_FORMAT_R8G8_UNORM ||
				format == VK_FORMAT_R8G8_SNORM ||
				format == VK_FORMAT_R8G8_USCALED ||
				format == VK_FORMAT_R8G8_SSCALED ||
				format == VK_FORMAT_R8G8_UINT ||
				format == VK_FORMAT_R8G8_SINT ||
				format == VK_FORMAT_R8G8_SRGB ||
				format == VK_FORMAT_R8G8B8_UNORM ||
				format == VK_FORMAT_R8G8B8_SNORM ||
				format == VK_FORMAT_R8G8B8_USCALED ||
				format == VK_FORMAT_R8G8B8_SSCALED ||
				format == VK_FORMAT_R8G8B8_UINT ||
				format == VK_FORMAT_R8G8B8_SINT ||
				format == VK_FORMAT_R8G8B8_SRGB ||
				format == VK_FORMAT_B8G8R8_UNORM ||
				format == VK_FORMAT_B8G8R8_SNORM ||
				format == VK_FORMAT_B8G8R8_USCALED ||
				format == VK_FORMAT_B8G8R8_SSCALED ||
				format == VK_FORMAT_B8G8R8_UINT ||
				format == VK_FORMAT_B8G8R8_SINT ||
				format == VK_FORMAT_B8G8R8_SRGB ||
				format == VK_FORMAT_R8G8B8A8_UNORM ||
				format == VK_FORMAT_R8G8B8A8_SNORM ||
				format == VK_FORMAT_R8G8B8A8_USCALED ||
				format == VK_FORMAT_R8G8B8A8_SSCALED ||
				format == VK_FORMAT_R8G8B8A8_UINT ||
				format == VK_FORMAT_R8G8B8A8_SINT ||
				format == VK_FORMAT_R8G8B8A8_SRGB ||
				format == VK_FORMAT_B8G8R8A8_UNORM ||
				format == VK_FORMAT_B8G8R8A8_SNORM ||
				format == VK_FORMAT_B8G8R8A8_USCALED ||
				format == VK_FORMAT_B8G8R8A8_SSCALED ||
				format == VK_FORMAT_B8G8R8A8_UINT ||
				format == VK_FORMAT_B8G8R8A8_SINT ||
				format == VK_FORMAT_B8G8R8A8_SRGB ||
				format == VK_FORMAT_A8B8G8R8_UNORM_PACK32 ||
				format == VK_FORMAT_A8B8G8R8_SNORM_PACK32 ||
				format == VK_FORMAT_A8B8G8R8_USCALED_PACK32 ||
				format == VK_FORMAT_A8B8G8R8_SSCALED_PACK32 ||
				format == VK_FORMAT_A8B8G8R8_UINT_PACK32 ||
				format == VK_FORMAT_A8B8G8R8_SINT_PACK32 ||
				format == VK_FORMAT_A8B8G8R8_SRGB_PACK32 ||
				format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
				format == VK_FORMAT_A2R10G10B10_SNORM_PACK32 ||
				format == VK_FORMAT_A2R10G10B10_USCALED_PACK32 ||
				format == VK_FORMAT_A2R10G10B10_SSCALED_PACK32 ||
				format == VK_FORMAT_A2R10G10B10_UINT_PACK32 ||
				format == VK_FORMAT_A2R10G10B10_SINT_PACK32 ||
				format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 ||
				format == VK_FORMAT_A2B10G10R10_SNORM_PACK32 ||
				format == VK_FORMAT_A2B10G10R10_USCALED_PACK32 ||
				format == VK_FORMAT_A2B10G10R10_SSCALED_PACK32 ||
				format == VK_FORMAT_A2B10G10R10_UINT_PACK32 ||
				format == VK_FORMAT_A2B10G10R10_SINT_PACK32 ||
				format == VK_FORMAT_R16_UNORM ||
				format == VK_FORMAT_R16_SNORM ||
				format == VK_FORMAT_R16_USCALED ||
				format == VK_FORMAT_R16_SSCALED ||
				format == VK_FORMAT_R16_UINT ||
				format == VK_FORMAT_R16_SINT ||
				format == VK_FORMAT_R16_SFLOAT ||
				format == VK_FORMAT_R16G16_UNORM ||
				format == VK_FORMAT_R16G16_SNORM ||
				format == VK_FORMAT_R16G16_USCALED ||
				format == VK_FORMAT_R16G16_SSCALED ||
				format == VK_FORMAT_R16G16_UINT ||
				format == VK_FORMAT_R16G16_SINT ||
				format == VK_FORMAT_R16G16_SFLOAT ||
				format == VK_FORMAT_R16G16B16_UNORM ||
				format == VK_FORMAT_R16G16B16_SNORM ||
				format == VK_FORMAT_R16G16B16_USCALED ||
				format == VK_FORMAT_R16G16B16_SSCALED ||
				format == VK_FORMAT_R16G16B16_UINT ||
				format == VK_FORMAT_R16G16B16_SINT ||
				format == VK_FORMAT_R16G16B16_SFLOAT ||
				format == VK_FORMAT_R16G16B16A16_UNORM ||
				format == VK_FORMAT_R16G16B16A16_SNORM ||
				format == VK_FORMAT_R16G16B16A16_USCALED ||
				format == VK_FORMAT_R16G16B16A16_SSCALED ||
				format == VK_FORMAT_R16G16B16A16_UINT ||
				format == VK_FORMAT_R16G16B16A16_SINT ||
				format == VK_FORMAT_R16G16B16A16_SFLOAT ||
				format == VK_FORMAT_R32_UINT ||
				format == VK_FORMAT_R32_SINT ||
				format == VK_FORMAT_R32_SFLOAT ||
				format == VK_FORMAT_R32G32_UINT ||
				format == VK_FORMAT_R32G32_SINT ||
				format == VK_FORMAT_R32G32_SFLOAT ||
				format == VK_FORMAT_R32G32B32_UINT ||
				format == VK_FORMAT_R32G32B32_SINT ||
				format == VK_FORMAT_R32G32B32_SFLOAT ||
				format == VK_FORMAT_R32G32B32A32_UINT ||
				format == VK_FORMAT_R32G32B32A32_SINT ||
				format == VK_FORMAT_R32G32B32A32_SFLOAT ||
				format == VK_FORMAT_R64_UINT ||
				format == VK_FORMAT_R64_SINT ||
				format == VK_FORMAT_R64_SFLOAT ||
				format == VK_FORMAT_R64G64_UINT ||
				format == VK_FORMAT_R64G64_SINT ||
				format == VK_FORMAT_R64G64_SFLOAT ||
				format == VK_FORMAT_R64G64B64_UINT ||
				format == VK_FORMAT_R64G64B64_SINT ||
				format == VK_FORMAT_R64G64B64_SFLOAT ||
				format == VK_FORMAT_R64G64B64A64_UINT ||
				format == VK_FORMAT_R64G64B64A64_SINT ||
				format == VK_FORMAT_R64G64B64A64_SFLOAT ||
				format == VK_FORMAT_B10G11R11_UFLOAT_PACK32 ||
				format == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 ||
				format == VK_FORMAT_BC1_RGB_UNORM_BLOCK ||
				format == VK_FORMAT_BC1_RGB_SRGB_BLOCK ||
				format == VK_FORMAT_BC1_RGBA_UNORM_BLOCK ||
				format == VK_FORMAT_BC1_RGBA_SRGB_BLOCK ||
				format == VK_FORMAT_BC2_UNORM_BLOCK ||
				format == VK_FORMAT_BC2_SRGB_BLOCK ||
				format == VK_FORMAT_BC3_UNORM_BLOCK ||
				format == VK_FORMAT_BC3_SRGB_BLOCK ||
				format == VK_FORMAT_BC4_UNORM_BLOCK ||
				format == VK_FORMAT_BC4_SNORM_BLOCK ||
				format == VK_FORMAT_BC5_UNORM_BLOCK ||
				format == VK_FORMAT_BC5_SNORM_BLOCK ||
				format == VK_FORMAT_BC6H_UFLOAT_BLOCK ||
				format == VK_FORMAT_BC6H_SFLOAT_BLOCK ||
				format == VK_FORMAT_BC7_UNORM_BLOCK ||
				format == VK_FORMAT_BC7_SRGB_BLOCK ||
				format == VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK ||
				format == VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK ||
				format == VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK ||
				format == VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK ||
				format == VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK ||
				format == VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK ||
				format == VK_FORMAT_EAC_R11_UNORM_BLOCK ||
				format == VK_FORMAT_EAC_R11_SNORM_BLOCK ||
				format == VK_FORMAT_EAC_R11G11_UNORM_BLOCK ||
				format == VK_FORMAT_EAC_R11G11_SNORM_BLOCK ||
				format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_4x4_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_5x4_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_5x4_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_5x5_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_5x5_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_6x5_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_6x5_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_6x6_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_6x6_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_8x5_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_8x5_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_8x6_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_8x6_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_8x8_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_8x8_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_10x5_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_10x5_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_10x6_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_10x6_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_10x8_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_10x8_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_10x10_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_10x10_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_12x10_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_12x10_SRGB_BLOCK ||
				format == VK_FORMAT_ASTC_12x12_UNORM_BLOCK ||
				format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK ||
				format == VK_FORMAT_G8B8G8R8_422_UNORM ||
				format == VK_FORMAT_B8G8R8G8_422_UNORM ||
				format == VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM ||
				format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM ||
				format == VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM ||
				format == VK_FORMAT_G8_B8R8_2PLANE_422_UNORM ||
				format == VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM ||
				format == VK_FORMAT_R10X6_UNORM_PACK16 ||
				format == VK_FORMAT_R10X6G10X6_UNORM_2PACK16 ||
				format == VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 ||
				format == VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 ||
				format == VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 ||
				format == VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 ||
				format == VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 ||
				format == VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 ||
				format == VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 ||
				format == VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 ||
				format == VK_FORMAT_R12X4_UNORM_PACK16 ||
				format == VK_FORMAT_R12X4G12X4_UNORM_2PACK16 ||
				format == VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 ||
				format == VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 ||
				format == VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 ||
				format == VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 ||
				format == VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 ||
				format == VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 ||
				format == VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 ||
				format == VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 ||
				format == VK_FORMAT_G16B16G16R16_422_UNORM ||
				format == VK_FORMAT_B16G16R16G16_422_UNORM ||
				format == VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM ||
				format == VK_FORMAT_G16_B16R16_2PLANE_420_UNORM ||
				format == VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM ||
				format == VK_FORMAT_G16_B16R16_2PLANE_422_UNORM ||
				format == VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM ||
				format == VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG ||
				format == VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG ||
				format == VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG ||
				format == VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG ||
				format == VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG ||
				format == VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG ||
				format == VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG ||
				format == VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG ||
				format == VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT ||
				format == VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT ||
				format == VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT
			) {
				return RenderPassAttachmentType::RENDERPASS_COLOR_ATTACHMENT;
			}
			if (
				format == VK_FORMAT_D16_UNORM ||
				format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
				format == VK_FORMAT_D32_SFLOAT ) {
				return RenderPassAttachmentType::RENDERPASS_DEPTH_ATTACHMENT;
			}
			if (format == VK_FORMAT_S8_UINT) {
				return RenderPassAttachmentType::RENDERPASS_STENCIL_ATTACHMENT;
			}
			if (
				format == VK_FORMAT_D16_UNORM_S8_UINT ||
				format == VK_FORMAT_D24_UNORM_S8_UINT ||
				format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
				return RenderPassAttachmentType::RENDERPASS_DEPTH_ATTACHMENT | RenderPassAttachmentType::RENDERPASS_STENCIL_ATTACHMENT;
			}
			return RenderPassAttachmentType::RENDERPASS_UNDEFINED_ATTACHMENT;
		}
	}
}
