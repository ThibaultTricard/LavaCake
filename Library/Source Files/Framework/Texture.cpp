#include "Texture.h"
#include "CommandBuffer.h"


namespace LavaCake {
	namespace Framework {

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																				Textrue Buffer 																																	//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		TextureBuffer::TextureBuffer(std::string filename, int nbChannel, VkFormat f) {
			int width, height;
			if (!Helpers::LoadTextureDataFromFile(filename.data(), nbChannel, *m_data, &width, &height)) {
				ErrorCheck::setError((char*)"Could not load texture file");
			}
			m_image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			m_image->createSampler();
			m_nbChannel = nbChannel;
		};

		TextureBuffer::TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat f) {
			m_data = data;
			m_image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			m_image->createSampler();
			m_format = f;
		};


		TextureBuffer::TextureBuffer(int width, int height, int nbChannel, VkFormat f) {
			m_data = nullptr;
			m_image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			m_image->createSampler();
			m_nbChannel = nbChannel;
			m_format = f;
		};

		void TextureBuffer::allocate(Queue* queue, CommandBuffer& commandBuffer, VkPipelineStageFlagBits stageFlagBit) {

			if (m_data != nullptr) {

				Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				VkPhysicalDevice physical = d->getPhysicalDevice();

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

				m_image->setLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);


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

				m_image->setLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, stageFlagBit, subresourceRange);


				commandBuffer.endRecord();


				commandBuffer.submit(queue, {}, {});



				commandBuffer.wait(UINT32_MAX);
				commandBuffer.resetFence();
			}
		}


		VkSampler& TextureBuffer::getSampler() {
			return m_image->getSampler();
		}

		VkImageView& TextureBuffer::getImageView() {
			return m_image->getImageView();
		}

		VkImageLayout TextureBuffer::getLayout() {
			return m_image->getLayout();
		}






		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																		Textrue Buffer 3D																																		//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		TextureBuffer3D::TextureBuffer3D(std::vector<unsigned char>* data, int width, int height, int depth, int nbChannel, VkFormat f) {
			m_data = data;
			m_image = new Image(width, height, depth, f, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

			m_nbChannel = nbChannel;
		};


		void TextureBuffer3D::allocate(Queue* queue, CommandBuffer& cmdBuff, VkPipelineStageFlagBits stageFlagBit) {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();


			m_image->createSampler();
			
			Buffer stagingBuffer;

			stagingBuffer.allocate(queue, cmdBuff, *m_data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);


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


			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, stageFlagBit, subresourceRange);


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

			m_image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);

			
		};

		void CubeMap::allocate(Queue* queue, CommandBuffer& cmdBuff, VkPipelineStageFlagBits stageFlagBit) {

			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();


			m_image->createSampler();
			
			
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
			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);

			stagingBuffer.copyToImage(cmdBuff, *m_image, { region });

			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, stageFlagBit, subresourceRange);

			cmdBuff.endRecord();

			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);

			cmdBuff.resetFence();
		}



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Attachement																																				//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Attachment::Attachment(int width, int height, VkFormat f, attachmentType type) {
			
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

			m_image = new Image(width, height, 1, f, aspect, (VkImageUsageFlagBits)(usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT));
		}


		Image* Attachment::getImage() {
			return m_image;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Storage Image																																			//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		StorageImage::StorageImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat f) {
			m_image = new Image(width, height, depth, f, VK_IMAGE_ASPECT_COLOR_BIT, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
		}

		void StorageImage::allocate(Queue* queue, CommandBuffer& cmdBuff) {

			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			m_image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, subresourceRange);

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


		Image* createAttachment(int width, int height, VkFormat f, attachmentType type) {

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

			return new Image(width, height, 1, f, aspect, (VkImageUsageFlagBits)(usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT));
		}


		Image* createStorageImage(Queue* queue, CommandBuffer& cmdBuff, int width, int height, int depth, VkFormat f) {
			Image* image = new Image(width, height, depth, f, VK_IMAGE_ASPECT_COLOR_BIT, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, subresourceRange);

			cmdBuff.endRecord();


			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);
			cmdBuff.resetFence();

			return image;
		}


		Image* createTextureBuffer(int width, int height, int nbChannel, VkFormat format) {
			return new Image(width, height, 1, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		}

		Image* createTextureBuffer(Queue* queue, CommandBuffer& cmdBuff, std::string filename, int nbChannel, VkFormat f, VkPipelineStageFlagBits stageFlagBit ) {
			int width, height;
			std::vector<unsigned char>* data = new std::vector<unsigned char>();
			if (!Helpers::LoadTextureDataFromFile(filename.data(), nbChannel, *data, &width, &height)) {
				ErrorCheck::setError((char*)"Could not load texture file");
			}
			
			return  createTextureBuffer(queue, cmdBuff, data, width, height, 1, nbChannel, f, stageFlagBit);
		}

		Image* createTextureBuffer(Queue* queue, CommandBuffer& cmdBuff, std::vector<unsigned char>* data, int width, int height, int depth, int nbChannel, VkFormat format, VkPipelineStageFlagBits stageFlagBit) {
			
			Image* image = new Image(width, height, depth, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			
			image->createSampler();

			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();

			VkBufferCreateInfo buffer_create_info = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,						// VkStructureType        sType
			nullptr,																				// const void           * pNext
			0,																							// VkBufferCreateFlags    flags
			static_cast<VkDeviceSize>(data->size()),      // VkDeviceSize           size
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,								// VkBufferUsageFlags     usage
			VK_SHARING_MODE_EXCLUSIVE,											// VkSharingMode          sharingMode
			0,																							// uint32_t               queueFamilyIndexCount
			nullptr																					// const uint32_t       * pQueueFamilyIndices
			};


			Buffer stagingBuffer;

			stagingBuffer.allocate(queue, cmdBuff, *data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			cmdBuff.beginRecord();

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);

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
						{ image->width(), image->height(), image->depth() },												// VkExtent3D                 imageExtent
			};

			stagingBuffer.copyToImage(cmdBuff, *image, { region });

			image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, stageFlagBit, subresourceRange);

			cmdBuff.endRecord();
			cmdBuff.submit(queue, {}, {});
			cmdBuff.wait(UINT32_MAX);
			cmdBuff.resetFence();
			return image;
		}

		Image* createCubeMap(Queue* queue, CommandBuffer& cmdBuff, std::string path, int nbChannel, std::vector<std::string> images, VkFormat f, VkPipelineStageFlagBits stageFlagBit) {

			std::vector<unsigned char> cubemap_image_data;
			int image_data_size;

			int width, height;
			std::vector<unsigned char> data;
			for (size_t i = 0; i < images.size(); ++i) {
				std::vector<unsigned char> cubemap_image_data;
				int image_data_size;
				if (!Helpers::LoadTextureDataFromFile((path + images[i]).c_str(), nbChannel, cubemap_image_data, &width, &height, nullptr, &image_data_size)) {
					ErrorCheck::setError((char*)"Could not load all texture file");
				}
				data.insert(data.end(), cubemap_image_data.begin(), cubemap_image_data.end());
			}

			Image*  image = new Image(width, height, 1, f, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);


			image->createSampler();
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			
			

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
						{ image->width(), image->height(), image->depth() },												// VkExtent3D                 imageExtent
			};

			VkImageSubresourceRange subresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,static_cast<uint32_t>(0), 6 };
			image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);

			stagingBuffer.copyToImage(cmdBuff, *image, { region });

			image->setLayout(cmdBuff, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, stageFlagBit, subresourceRange);

			cmdBuff.endRecord();

			cmdBuff.submit(queue, {}, {});

			cmdBuff.wait(UINT32_MAX);

			cmdBuff.resetFence();

			return image;
		};

	}
}
