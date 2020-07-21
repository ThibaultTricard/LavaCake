#include "Texture.h"

namespace LavaCake {
	namespace Framework {



		TextureBuffer::TextureBuffer(char const * filename, int nbChannel, VkFormat f) {
			if (!Helpers::Texture::LoadTextureDataFromFile(filename, nbChannel, *m_data, &m_width, &m_height)) {
				ErrorCheck::setError("Could not load texture file");
			}
			m_format = f;
		};

		TextureBuffer::TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat f) {
			m_data = data;
			m_width = width;
			m_height = height;
			m_nbChannel = nbChannel;
			m_format = f;
		};


		void TextureBuffer::allocate() {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();
			Buffer::FrameResources& frame = d->getFrameRessources()->at(0);
			VkCommandBuffer commandbuffer = frame.CommandBuffer;
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
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, graphics_queue, commandbuffer, {})) {
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

		FrameBuffer::FrameBuffer(int width, int height, VkFormat f) {
			m_width = width;
			m_height = height;
			m_format = f;
		};

		void FrameBuffer::allocate() {

			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();

			InitVkDestroyer(logical, m_sampler);
			InitVkDestroyer(logical, m_image);
			InitVkDestroyer(logical, m_imageMemory);
			InitVkDestroyer(logical, m_imageView);


			if (!Image::CreateCombinedImageSampler(physical, logical, VK_IMAGE_TYPE_2D, m_format, { (uint32_t)m_width, (uint32_t)m_height, 1 }, 1, 1,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, false, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, VK_FILTER_LINEAR,
				VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
				false, *m_sampler, *m_image, *m_imageMemory, *m_imageView)) {
				ErrorCheck::setError("Can't create an image sampler for this FrameBuffer");
			}
		}

		void FrameBuffer::setInputRenderPass(VkRenderPass pass) {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			InitVkDestroyer(logical, m_frameBuffer);

			if (!Buffer::CreateFramebuffer(logical, pass, { *m_imageView }, m_width, m_height, 1, *m_frameBuffer)) {
				return;
			}

		}

		VkImageLayout FrameBuffer::getLayout(){
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
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

			if (!Helpers::Texture::LoadTextureDataFromFile((m_path + m_images[0]).c_str(), 4, cubemap_image_data, &m_width, &m_height, nullptr, &image_data_size)) {
				ErrorCheck::setError("Could not locate texture file");
			}
		};

		void CubeMap::allocate() {

			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();
			Buffer::FrameResources& frame = d->getFrameRessources()->at(0);
			VkCommandBuffer commandbuffer = frame.CommandBuffer;


			InitVkDestroyer(logical, m_image);
			InitVkDestroyer(logical, m_imageMemory);
			InitVkDestroyer(logical, m_imageView);
			InitVkDestroyer(logical, m_sampler);
			if (!Image::CreateCombinedImageSampler(physical, logical, VK_IMAGE_TYPE_2D, m_format, { uint32_t(m_width), uint32_t(m_height), 1 }, 1, 6,
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_LINEAR,
				VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
				false, *m_sampler, *m_image, *m_imageMemory, *m_imageView)) {
				ErrorCheck::setError("Can't create an image sampler for this CubeMap");
			}


			for (size_t i = 0; i < m_images.size(); ++i) {
				std::vector<unsigned char> cubemap_image_data;
				int image_data_size;
				if (!Helpers::Texture::LoadTextureDataFromFile((m_path + m_images[i]).c_str(), m_nbChannel, cubemap_image_data, nullptr, nullptr, nullptr, &image_data_size)) {
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
					0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					graphics_queue, commandbuffer, {});
			}
		}



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//																																			Attachement																																				//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Attachment::Attachment(int width, int height, VkFormat f) {
			m_width = width;
			m_height = height;
			m_format = f;
		}

		void Attachment::allocate() {
			Framework::Device* d = LavaCake::Framework::Device::getDevice();
			VkDevice logical = d->getLogicalDevice();

			InitVkDestroyer(logical, m_image);
			InitVkDestroyer(logical, m_imageMemory);
			InitVkDestroyer(logical, m_imageView);
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();

			if (!Image::CreateInputAttachment(physical, logical, VK_IMAGE_TYPE_2D, m_format, { (uint32_t)m_width,
				(uint32_t)m_height, 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_VIEW_TYPE_2D,
				VK_IMAGE_ASPECT_COLOR_BIT, *m_image, *m_imageMemory, *m_imageView)) {
				ErrorCheck::setError("Can't allocate this input Attachment");
			}
		}

		VkImageLayout Attachment::getLayout() {
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkImageView Attachment::getImageView() {
			return *m_imageView;
		}
	}
}