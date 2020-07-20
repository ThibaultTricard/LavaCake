#pragma once
#include <iostream>
#include <vector>
#include "AllHeaders.h"
#include "Device.h"
#include "Texture.h"


namespace LavaCake{
	namespace Framework {
		class CubeMap : public TextureBuffer{
		public :
			CubeMap(std::string path, int nbChannel, std::vector<std::string> images = {"posx.jpg","negx.jpg","posy.jpg","negy.jpg","posz.jpg","negz.jpg"}, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM) {
				m_path = path;
				m_images = images;
				m_format = f;
				m_nbChannel = nbChannel;

				std::vector<unsigned char> cubemap_image_data;
				int image_data_size;

				if (!Helpers::Texture::LoadTextureDataFromFile((m_path + m_images[0]).c_str(), 4, cubemap_image_data, &m_width, &m_height, nullptr, &image_data_size)) {
					//return false;
				}

				
			};

			virtual void compile() override {

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
					//return false;
				}


				for (size_t i = 0; i < m_images.size(); ++i) {
					std::vector<unsigned char> cubemap_image_data;
					int image_data_size;
					if (!Helpers::Texture::LoadTextureDataFromFile((m_path + m_images[i]).c_str(), m_nbChannel, cubemap_image_data, nullptr, nullptr, nullptr, &image_data_size)) {
						//return false;
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

		private:


			std::string													m_path;
			std::vector<std::string>						m_images;
		};
	}
}