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

			TextureBuffer(char const * filename, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);

			TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);

			virtual void allocate();

			VkSampler	getSampler();

			VkImageView getImageView();

			virtual VkImageLayout getLayout();

		protected :

			TextureBuffer() {};

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

		class CubeMap : public TextureBuffer {
		public:
			CubeMap(std::string path, int nbChannel, std::vector<std::string> images = { "posx.jpg","negx.jpg","posy.jpg","negy.jpg","posz.jpg","negz.jpg" }, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);

			virtual void allocate() override;

		private:
			std::string													m_path;
			std::vector<std::string>						m_images;
		};

		class FrameBuffer : public TextureBuffer {
		public : 
			
			FrameBuffer(int width, int height, VkFormat f);

			virtual void allocate() override;
				
			virtual void setInputRenderPass(VkRenderPass pass);

			virtual VkImageLayout getLayout() override;

			VkFramebuffer getFrameBuffer();


		private :

			VkDestroyer(VkFramebuffer) m_frameBuffer;

		};

		class Attachment {
		public :

			Attachment(int width, int height, VkFormat f);

			void allocate();

			virtual VkImageLayout getLayout();

			VkImageView getImageView();

		private : 

			int																	m_width = 0;
			int																	m_height = 0;
			VkFormat														m_format;

			VkDestroyer(VkImage)                m_image;
			VkDestroyer(VkDeviceMemory)         m_imageMemory;
			VkDestroyer(VkImageView)            m_imageView;
		};

	}
}