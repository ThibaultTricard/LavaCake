#include "Texture.h"
#include "CommandBuffer.h"

namespace LavaCake {
	namespace Framework {


		
		TextureBuffer::TextureBuffer(char const * filename, int nbChannel, VkFormat f) {
			int width, height;
			if (!Helpers::LoadTextureDataFromFile(filename, nbChannel, *m_data, &width, &height)) {
				ErrorCheck::setError("Could not load texture file");
			}
			m_width = width;
			m_height = height;
			m_format = f;
		};

		TextureBuffer::TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat f) {
			m_data = data;
			m_width = width;
			m_height = height;
			m_nbChannel = nbChannel;
			m_format = f;
		};


		void TextureBuffer::allocate(VkQueue& queue, VkCommandBuffer& commandBuffer, VkPipelineStageFlagBits stageFlagBit) {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();

			InitVkDestroyer(logical, m_sampler);
			InitVkDestroyer(logical, m_image);
			InitVkDestroyer(logical, m_imageMemory);
			InitVkDestroyer(logical, m_imageView);
			if (!Image::CreateCombinedImageSampler(physical, logical, VK_IMAGE_TYPE_2D, m_format, { (uint32_t)m_width, (uint32_t)m_height, 1 },
				1, 1, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_LINEAR,
				VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
				VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
				false, *m_sampler, *m_image, *m_imageMemory, *m_imageView)) {
				ErrorCheck::setError("Can't create an image sampler for this TextureBuffer");
			}

			VkImageSubresourceLayers image_subresource_layer = {
				VK_IMAGE_ASPECT_COLOR_BIT,    // VkImageAspectFlags     aspectMask
				0,                            // uint32_t               mipLevel
				0,                            // uint32_t               baseArrayLayer
				1                             // uint32_t               layerCount
			};
			if (!Memory::UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(physical, logical, static_cast<VkDeviceSize>(m_data->size()),
				&(*m_data)[0], *m_image, image_subresource_layer, { 0, 0, 0 }, { (uint32_t)m_width, (uint32_t)m_height, 1 }, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				stageFlagBit, queue, commandBuffer, {})) {
				ErrorCheck::setError("Can't send the TextureBuffer data to the GPU");
			}
		}


		VkSampler	TextureBuffer::getSampler() {
			return *m_sampler;
		}

		VkImageView TextureBuffer::getImageView() {
			return *m_imageView;
		}

		VkImageLayout TextureBuffer::getLayout() {
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																				Frame Buffer																																		//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		FrameBuffer::FrameBuffer(uint32_t width, uint32_t height) {
			m_width = width;
			m_height = height;
		};

		VkImageView FrameBuffer::getImageViews(int i) {
			return *m_imageViews[i];
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

		VkFramebuffer FrameBuffer::getFrameBuffer() {
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
				ErrorCheck::setError("Could not locate texture file");
			}

			m_width = width;
			m_height = height;
		};

		void CubeMap::allocate(VkQueue& queue, VkCommandBuffer& commandBuffer, VkPipelineStageFlagBits stageFlagBit) {

			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();


			InitVkDestroyer(logical, m_image);
			InitVkDestroyer(logical, m_imageMemory);
			InitVkDestroyer(logical, m_imageView);
			InitVkDestroyer(logical, m_sampler);
			if (!Image::CreateCombinedImageSampler(physical, logical, VK_IMAGE_TYPE_2D, m_format, { uint32_t(m_width), uint32_t(m_height), 1 }, 1, 1,
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_LINEAR,
				VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
				false, *m_sampler, *m_image, *m_imageMemory, *m_imageView)) {
				ErrorCheck::setError("Can't create an image sampler for this CubeMap");
			}


			for (size_t i = 0; i < m_images.size(); ++i) {
				std::vector<unsigned char> cubemap_image_data;
				int image_data_size;
				if (!Helpers::LoadTextureDataFromFile((m_path + m_images[i]).c_str(), m_nbChannel, cubemap_image_data, nullptr, nullptr, nullptr, &image_data_size)) {
					ErrorCheck::setError("Could not load all texture file");
				}
				VkImageSubresourceLayers image_subresource = {
					VK_IMAGE_ASPECT_COLOR_BIT,    // VkImageAspectFlags     aspectMask
					0,                            // uint32_t               mipLevel
					static_cast<uint32_t>(i),     // uint32_t               baseArrayLayer
					1                             // uint32_t               layerCount
				};
				Memory::UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(physical, logical, image_data_size, &cubemap_image_data[0],
					*m_image, image_subresource, { 0, 0, 0 }, { uint32_t(m_width), uint32_t(m_height), 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, stageFlagBit,
					queue, commandBuffer, {});
			}
		}



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Attachement																																				//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Attachment::Attachment(int width, int height, VkFormat f, attachmentType type) {
			m_width = width;
			m_height = height;
			m_format = f;
			m_type = type;
		}

		void Attachment::allocate() {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();

			InitVkDestroyer(logical, m_image);
			InitVkDestroyer(logical, m_imageMemory);
			InitVkDestroyer(logical, m_imageView);
			VkPhysicalDevice physical = d->getPhysicalDevice();

			VkImageUsageFlagBits usage;
			VkImageAspectFlagBits aspect;
			if (m_type == COLOR_ATTACHMENT) {
				usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			else if(m_type == DEPTH_ATTACHMENT) {
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else {
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
			}

			if (!Image::CreateImage(logical, VK_IMAGE_TYPE_2D, m_format, { (uint32_t)m_width,
				(uint32_t)m_height, 1 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, false, *m_image)) {
				ErrorCheck::setError("Can't create an Image for this Attachment");
			}

			if (!Image::AllocateAndBindMemoryObjectToImage(physical, logical, *m_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_imageMemory)) {
				ErrorCheck::setError("Can't create a Memory object for this Attachment");
			}

			if (!Image::CreateImageView(logical, *m_image, VK_IMAGE_VIEW_TYPE_2D, m_format, aspect, *m_imageView)) {
				ErrorCheck::setError("Can't create an Image view for this Attachment");
			}

		}

		VkImageLayout Attachment::getLayout() {
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkImageView Attachment::getImageView() {
			return *m_imageView;
		}


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Storage Image																																			//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		StorageImage::StorageImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat f) {
			m_width = width;
			m_height = height;
			m_depth = depth;
			m_format = f;
		}

		void StorageImage::allocate() {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();

			InitVkDestroyer(logical, m_image);
			InitVkDestroyer(logical, m_imageMemory);
			InitVkDestroyer(logical, m_imageView);
			VkPhysicalDevice physical = d->getPhysicalDevice();

			

			VkImageType type = VK_IMAGE_TYPE_1D;
			VkImageViewType view = VK_IMAGE_VIEW_TYPE_1D;
			if (m_height > 1) { type = VK_IMAGE_TYPE_2D; view = VK_IMAGE_VIEW_TYPE_2D; }
			if (m_depth > 1) { type = VK_IMAGE_TYPE_3D; view = VK_IMAGE_VIEW_TYPE_3D; }
			
			if (!Image::CreateImage(logical, type, m_format, { m_width, m_height, m_depth }, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, *m_image)) {
				ErrorCheck::setError("Can't create Image");
			}

			if (!Image::AllocateAndBindMemoryObjectToImage(physical, logical, *m_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_imageMemory)) {
				ErrorCheck::setError("Can't allocate Image memory");
			}

			if (!Image::CreateImageView(logical, *m_image, view, m_format, VK_IMAGE_ASPECT_COLOR_BIT, *m_imageView)) {
				ErrorCheck::setError("Can't create Image View");
			}
		}

		VkImageLayout StorageImage::getLayout() {
			return VK_IMAGE_LAYOUT_GENERAL;
		}

		VkImageView StorageImage::getImageView() {
			return *m_imageView;
		}



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Storage Image																																			//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		TexelBuffer::TexelBuffer() {

		}


		void TexelBuffer::allocate(VkQueue& queue, VkCommandBuffer& commandBuffer ,std::vector<float> rawdata, uint32_t dataSize, VkPipelineStageFlagBits stageFlagBit) {
			Device* d = Device::getDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkDevice logical = d->getLogicalDevice();

			VkFormat format = VK_FORMAT_R32_SFLOAT;
			if (dataSize == 2) {
				format = VK_FORMAT_R32G32_SFLOAT;
			}
			else if (dataSize == 3) {
				format = VK_FORMAT_R32G32B32_SFLOAT;
			}
			else if (dataSize == 4) {
				format = VK_FORMAT_R32G32B32A32_SFLOAT;
			}

			if (!LavaCake::Buffer::CreateStorageTexelBuffer(physical, logical, format, sizeof(rawdata[0]) * rawdata.size(),
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | stageFlagBit, false,
				*m_buffer, *m_bufferMemory, *m_bufferView)) {
				ErrorCheck::setError("Can't create Texel Buffer");
			}

			if (!LavaCake::Memory::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physical, logical, sizeof(rawdata[0]) * rawdata.size(), &rawdata[0],
				*m_buffer, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT , queue, commandBuffer, {})) {
				ErrorCheck::setError("Can't copy data to buffer");
			}
		}

		VkBuffer TexelBuffer::getBuffer() {
			return *m_buffer;
		}

		VkBufferView TexelBuffer::getBufferView() {
			return *m_bufferView;
		}
	}
}