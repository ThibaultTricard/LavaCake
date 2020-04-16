#pragma once

#include "AllHeaders.h"
#include "VulkanDestroyer.h"
#include "Queue.h"
#include "Device.h"
#include "ErrorCheck.h"

namespace LavaCake {
	namespace Framework {
		class TextureBuffer {
		public:
			TextureBuffer(char const * filename, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM) {
				if (!Helpers::Texture::LoadTextureDataFromFile(filename, nbChannel, *m_data, &m_width, &m_height)) {
					ErrorCheck::setError(15,"could not load texture file");
				}
				m_format = f;
			};

			TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM) {
				m_data = data;
				m_width = width;
				m_height = height;
				m_nbChannel = nbChannel;
				m_format = f;
			};


			void compile() {
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
					//return false;
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
					//return false;
				}
			}
		

			VkSampler	getSampler() {
				return *m_sampler;
			}

			VkImageView getImageView() {
				return *m_imageView;
			}

		private :

			int																	m_width = 0;
			int																	m_height = 0;
			int																	m_nbChannel = 0;
			std::vector<unsigned char>*					m_data = new std::vector<unsigned char>();


			VkDestroyer(VkImage)                m_image;
			VkDestroyer(VkDeviceMemory)         m_imageMemory;
			VkDestroyer(VkImageView)            m_imageView;
			VkDestroyer(VkSampler)              m_sampler;


			VkFormat														m_format;
		};
	}
}