#include "Texture.h"
#include "CommandBuffer.h"


namespace LavaCake {
	namespace Framework {


		Image createAttachment(Queue& queue, CommandBuffer& cmdBuff, int width, int height, VkFormat f, attachmentType type) {

			VkImageAspectFlagBits aspect;
			if (type == COLOR_ATTACHMENT) {
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			else if (type == DEPTH_ATTACHMENT) {
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else {
				aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
			}

			VkImageUsageFlagBits usage;
			if (type == COLOR_ATTACHMENT) {
				usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
			else if (type == DEPTH_ATTACHMENT) {
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else {
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}

			Image image(width, height, 1, f, aspect, (VkImageUsageFlagBits)(usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT));

			cmdBuff.beginRecord();
			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			image.setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresourceRange);

			cmdBuff.endRecord();

			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);
			cmdBuff.resetFence();

			return std::move(image);
		}


		Image createStorageImage(Queue& queue, CommandBuffer& cmdBuff, int width, int height, int depth, VkFormat f) {
			Image image(width, height, depth, f, VK_IMAGE_ASPECT_COLOR_BIT, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			image.setLayout(cmdBuff, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, subresourceRange);

			cmdBuff.endRecord();


			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);
			cmdBuff.resetFence();

			return std::move(image);
		}


		Image createTextureBuffer(int width, int height, int depth, VkFormat format) {
			return Image(width, height, depth, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		}

		Image createTextureBuffer(Queue& queue, CommandBuffer& cmdBuff, const std::string& filename, int nbChannel, VkFormat f, VkPipelineStageFlagBits stageFlagBit) {
			int width, height;
			std::vector<unsigned char> data = std::vector<unsigned char>();
			if (!Helpers::LoadTextureDataFromFile(filename.data(), nbChannel, data, &width, &height)) {
				ErrorCheck::setError("Could not load texture file");
			}

			return createTextureBuffer(queue, cmdBuff, data, width, height, 1, nbChannel, f, stageFlagBit);
		}

		Image createTextureBuffer(Queue& queue, CommandBuffer& cmdBuff,const std::vector<unsigned char>& data, int width, int height, int depth, int nbChannel, VkFormat format, VkPipelineStageFlagBits stageFlagBit) {

			Image image(width, height, depth, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

			image.createSampler();

			Buffer stagingBuffer;

			stagingBuffer.allocate(queue, cmdBuff, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			image.setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);

			VkImageSubresourceLayers image_subresource_layer = {
				VK_IMAGE_ASPECT_COLOR_BIT,    // VkImageAspectFlags     aspectMask
				0,                            // uint32_t               mipLevel
				0,                            // uint32_t               baseArrayLayer
				1                             // uint32_t               layerCount
			};

			VkBufferImageCopy region = {
						0,																																								// VkDeviceSize               bufferOffset
						0,																																								// uint32_t                   bufferRowLength
						0,																																								// uint32_t                   bufferImageHeight
						image_subresource_layer,																													// VkImageSubresourceLayers   imageSubresource
						{ 0, 0, 0 },																																			// VkOffset3D                 imageOffset
						{ image.width(), image.height(), image.depth() },																	// VkExtent3D                 imageExtent
			};

			stagingBuffer.copyToImage(cmdBuff, image, { region });

			image.setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, stageFlagBit, subresourceRange);

			cmdBuff.endRecord();
			cmdBuff.submit(queue, {}, {});
			cmdBuff.wait(UINT32_MAX);
			cmdBuff.resetFence();
			return std::move(image);
		}

		Image createCubeMap(Queue& queue, CommandBuffer& cmdBuff,const std::string& path, int nbChannel,const std::array<std::string,6>& images, VkFormat f, VkPipelineStageFlagBits stageFlagBit) {

			std::vector<unsigned char> cubemap_image_data;

			int width, height;
			std::vector<unsigned char> data;
			for (size_t i = 0; i < images.size(); ++i) {
				std::vector<unsigned char> cubemap_image_data;
				int image_data_size;
				if (!Helpers::LoadTextureDataFromFile((path + images[i]).c_str(), nbChannel, cubemap_image_data, &width, &height, nullptr, &image_data_size)) {
					ErrorCheck::setError("Could not load all texture file");
				}
				data.insert(data.end(), cubemap_image_data.begin(), cubemap_image_data.end());
			}

			Image image = Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);

			image.createSampler();

			Buffer stagingBuffer;

			stagingBuffer.allocate(queue, cmdBuff, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			cmdBuff.beginRecord();

			VkImageSubresourceLayers image_subresource_layer = {
					VK_IMAGE_ASPECT_COLOR_BIT,    // VkImageAspectFlags     aspectMask
					0,                            // uint32_t               mipLevel
					static_cast<uint32_t>(0),     // uint32_t               baseArrayLayer
					6                             // uint32_t               layerCount
			};

			VkBufferImageCopy region = {
						0,																																								// VkDeviceSize               bufferOffset
						0,																																								// uint32_t                   bufferRowLength
						0,																																								// uint32_t                   bufferImageHeight
						image_subresource_layer,																													// VkImageSubresourceLayers   imageSubresource
						{ 0, 0, 0 },																																			// VkOffset3D                 imageOffset
						{ image.width(), image.height(), image.depth() },															// VkExtent3D                 imageExtent
			};

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,static_cast<uint32_t>(0), 6 };
			image.setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);

			stagingBuffer.copyToImage(cmdBuff, image, { region });

			image.setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, stageFlagBit, subresourceRange);

			cmdBuff.endRecord();

			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);

			cmdBuff.resetFence();

			return std::move(image);
		};



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																				Frame Buffer																																		//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		FrameBuffer::FrameBuffer(uint32_t width, uint32_t height) {
			m_width = width;
			m_height = height;
		};

		VkImageView& FrameBuffer::getImageViews(uint8_t i) {
			return m_images[i]->getImageView();
		}

		size_t FrameBuffer::getImageViewSize(){
			return m_images.size();
		}

		VkImageLayout& FrameBuffer::getLayout(uint8_t i){
			return m_images[i]->getLayout();
		}

		VkSampler&	FrameBuffer::getSampler() {
			return m_sampler;
		}

		VkFramebuffer& FrameBuffer::getHandle() {
			return m_frameBuffer;
		}




	}
}
