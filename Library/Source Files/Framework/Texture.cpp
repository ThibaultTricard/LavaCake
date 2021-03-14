#include "Texture.h"
#include "CommandBuffer.h"


namespace LavaCake {
	namespace Framework {

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																				Textrue Buffer 																																	//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		TextureBuffer::TextureBuffer(char const * filename, int nbChannel, VkFormat f) {
			int width, height;
			if (!Helpers::LoadTextureDataFromFile(filename, nbChannel, *m_data, &width, &height)) {
				ErrorCheck::setError((char*)"Could not load texture file");
			}
			m_image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT);
		
			m_nbChannel = nbChannel;
		};

		TextureBuffer::TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat f) {
			m_data = data;
			m_image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT);
			m_nbChannel = nbChannel;
			m_format = f;
		};


		void TextureBuffer::allocate(Queue* queue, CommandBuffer& commandBuffer, VkPipelineStageFlagBits stageFlagBit) {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();


			InitVkDestroyer(logical, m_sampler);
			if (!LavaCake::Core::CreateSampler(logical, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, false, *m_sampler)) {
				//return false;
			}

			m_image->allocate(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			

			VkDestroyer(VkBuffer) staging_buffer;
			VkBufferCreateInfo buffer_create_info = {
				VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,						// VkStructureType        sType
				nullptr,																				// const void           * pNext
				0,																							// VkBufferCreateFlags    flags
				static_cast<VkDeviceSize>(m_data->size()),      // VkDeviceSize           size
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,								// VkBufferUsageFlags     usage
				VK_SHARING_MODE_EXCLUSIVE,											// VkSharingMode          sharingMode
				0,																							// uint32_t               queueFamilyIndexCount
				nullptr																					// const uint32_t       * pQueueFamilyIndices
			};


			Buffer stagingBuffer;

			stagingBuffer.allocate(queue, commandBuffer, *m_data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			commandBuffer.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			m_image->setLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT);


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
						{ m_image->width(), m_image->height(), m_image->depth() },												// VkExtent3D                 imageExtent
			};

			stagingBuffer.copyToImage(commandBuffer, *m_image, { region });

			m_image->setLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, stageFlagBit);

			
			commandBuffer.endRecord();
			

			commandBuffer.submit(queue, {}, {});



			commandBuffer.wait(UINT32_MAX);
			commandBuffer.resetFence();

		}


		VkSampler	TextureBuffer::getSampler() {
			return *m_sampler;
		}

		VkImageView TextureBuffer::getImageView() {
			return m_image->getImageView();
		}

		VkImageLayout TextureBuffer::getLayout() {
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}






		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																		Textrue Buffer 3D																																		//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



		TextureBuffer3D::TextureBuffer3D(std::vector<unsigned char>* data, int width, int height, int depth, int nbChannel, VkFormat f) {
			m_data = data;
			m_image = new Image(width, height, depth, f, VK_IMAGE_ASPECT_COLOR_BIT);

			m_nbChannel = nbChannel;
		};


		void TextureBuffer3D::allocate(Queue* queue, CommandBuffer& cmdBuff, VkPipelineStageFlagBits stageFlagBit) {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();

			InitVkDestroyer(logical, m_sampler);

			if (!LavaCake::Core::CreateSampler(logical, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, false, *m_sampler)) {
				//return false;
			}

			m_image->allocate(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

			
			Buffer stagingBuffer;

			stagingBuffer.allocate(queue, cmdBuff, *m_data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT);


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
						{ m_image->width(), m_image->height(), m_image->depth() },												// VkExtent3D                 imageExtent
			};

			stagingBuffer.copyToImage(cmdBuff, *m_image, { region });


			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, stageFlagBit);


			cmdBuff.endRecord();

			cmdBuff.submit(queue, {}, {});



			cmdBuff.wait(UINT32_MAX);
			cmdBuff.resetFence();

		}



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																				Frame Buffer																																		//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		FrameBuffer::FrameBuffer(uint32_t width, uint32_t height) {
			m_width = width;
			m_height = height;
		};

		VkImageView FrameBuffer::getImageViews(int i) {
			return m_imageViews[i];
		}

		size_t FrameBuffer::getImageViewSize(){
			return m_imageViews.size();
		}

		VkImageLayout FrameBuffer::getLayout(int i){
			return m_layouts[i];
		}

		VkSampler	FrameBuffer::getSampler() {
			return *m_sampler;
		}

		VkFramebuffer FrameBuffer::getHandle() {
			return *m_frameBuffer;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																					 CubeMap																																			//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		CubeMap::CubeMap(std::string path, int nbChannel, std::vector<std::string> images, VkFormat f) {
			m_path = path;
			m_images = images;
			m_format = f;
			m_nbChannel = nbChannel;

			std::vector<unsigned char> cubemap_image_data;
			int image_data_size;

			int width, height;
			if (!Helpers::LoadTextureDataFromFile((m_path + m_images[0]).c_str(), 4, cubemap_image_data, &width, &height, nullptr, &image_data_size)) {
				ErrorCheck::setError((char*)"Could not locate texture file");
			}

			m_image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT, true);

			
		};

		void CubeMap::allocate(Queue* queue, CommandBuffer& cmdBuff, VkPipelineStageFlagBits stageFlagBit) {

			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();


			
			InitVkDestroyer(logical, m_sampler);

			if (!LavaCake::Core::CreateSampler(logical, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, false, *m_sampler)) {
				//return false;
			}

			m_image->allocate(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

			

			std::vector<unsigned char> data;
			for (size_t i = 0; i < m_images.size(); ++i) {
				std::vector<unsigned char> cubemap_image_data;
				int image_data_size;
				if (!Helpers::LoadTextureDataFromFile((m_path + m_images[i]).c_str(), m_nbChannel, cubemap_image_data, nullptr, nullptr, nullptr, &image_data_size)) {
					ErrorCheck::setError((char*)"Could not load all texture file");
				}
				data.insert(data.end(), cubemap_image_data.begin(), cubemap_image_data.end());
			}

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
						{ m_image->width(), m_image->height(), m_image->depth() },												// VkExtent3D                 imageExtent
			};

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,static_cast<uint32_t>(0), 6 };
			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT);

			stagingBuffer.copyToImage(cmdBuff, *m_image, { region });

			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, stageFlagBit);

			cmdBuff.endRecord();

			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);

			cmdBuff.resetFence();
		}



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Attachement																																				//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Attachment::Attachment(int width, int height, VkFormat f, attachmentType type) {
			
			m_type = type;
			
			VkImageAspectFlagBits aspect;
			if (m_type == COLOR_ATTACHMENT) {
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			else if (m_type == DEPTH_ATTACHMENT) {
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else {
				aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			m_image = new Image(width, height, 1, f, aspect);
		}

		void Attachment::allocate() {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();



			VkPhysicalDevice physical = d->getPhysicalDevice();

			VkImageUsageFlagBits usage;
			if (m_type == COLOR_ATTACHMENT) {
				usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
			else if(m_type == DEPTH_ATTACHMENT) {
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else {
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}

			m_image->allocate((VkImageUsageFlagBits)(usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT));


		}

		VkImageLayout Attachment::getLayout() {
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkImageView Attachment::getImageView() {
			return m_image->getImageView();
		}


		VkImage Attachment::getImage() {
			return m_image->getImage();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Storage Image																																			//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		StorageImage::StorageImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat f) {
			m_image = new Image(width, height, depth, f, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		void StorageImage::allocate(Queue* queue, CommandBuffer& cmdBuff) {


			m_image->allocate((VkImageUsageFlagBits)(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));

			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_GENERAL, subresourceRange, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

			cmdBuff.endRecord();

			

			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);
			cmdBuff.resetFence();
		}

		VkImageLayout StorageImage::getLayout() {
			return m_image->getLayout();
		}

		VkImageView StorageImage::getImageView() {
			return m_image->getImageView();
		}



		
	}
}
