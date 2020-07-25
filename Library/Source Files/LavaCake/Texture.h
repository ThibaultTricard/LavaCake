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


			/**
			* construct a texture buffer from a file.
			*
			* @param filename : the name of the file where the data is stored.
			* @param int nbChannel : the number of channel that contain the texture between 1 and 4.
			* @param VkFormat f : the format of theTexture buffer.
      *
			*/
			TextureBuffer(char const * filename, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);


			/**
			* construct a texture buffer from a array of data.
			*
			* @param data : the data that will be used of the texture.
			* @param int width : the width of the texture.
			* @param int height : the height of the texture.
			* @param int nbChannel : the number of channel that contain the texture between 1 and 4.
			* @param VkFormat f : the format of theTexture buffer.
			*
			*/
			TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);


			/**
			* create and allocate the Texture buffer on the device
			*
			*/
			virtual void allocate();

			/**
			* return the sampler of the texture buffer
			*
			*/
			VkSampler	getSampler();

			/**
			* return the image view  of the texture buffer
			*
			*/
			VkImageView getImageView();

			/**
			* return the layout the texture buffer
			*
			*/
			virtual VkImageLayout getLayout();

		protected :

			TextureBuffer() {};

			uint32_t																	m_width = 0;
			uint32_t																	m_height = 0;
			uint32_t																	m_nbChannel = 0;
			std::vector<unsigned char>*								m_data = new std::vector<unsigned char>();


			VkDestroyer(VkImage)											m_image;
			VkDestroyer(VkDeviceMemory)								m_imageMemory;
			VkDestroyer(VkImageView)									m_imageView;
			VkDestroyer(VkSampler)										m_sampler;


			VkFormat																	m_format;
		};

		class CubeMap : public TextureBuffer {
		public:
			CubeMap(std::string path, int nbChannel, std::vector<std::string> images = { "posx.jpg","negx.jpg","posy.jpg","negy.jpg","posz.jpg","negz.jpg" }, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);

			virtual void allocate() override;

		private:
			std::string													m_path;
			std::vector<std::string>						m_images;
		};


		enum frameBufferType {
			COLOR_FRAMEBUFFER, DEPTH_FRAMEBUFFER, STENCIL_FRAMEBUFFER
		};

		class FrameBuffer : public TextureBuffer {
		public : 
			
			FrameBuffer(uint32_t width, uint32_t height, uint32_t layer, VkFormat f, frameBufferType type);

			virtual void allocate() override;
				
			virtual void setInputRenderPass(VkRenderPass pass);

			virtual VkImageLayout getLayout() override;

			VkFramebuffer getFrameBuffer();


		private :

			frameBufferType											m_type;
			uint32_t														m_layer;
			VkDestroyer(VkFramebuffer)					m_frameBuffer;

		};

		enum attatchmentType {
			COLOR_ATTACHMENT ,DEPTH_ATTACHMENT, STENCIL_ATTACHMENT
		};

		class Attachment {
		public :

			Attachment(int width, int height, VkFormat f, attatchmentType type);

			void allocate();

			virtual VkImageLayout getLayout();

			VkImageView getImageView();

		private : 

			int																	m_width = 0;
			int																	m_height = 0;
			VkFormat														m_format;
			attatchmentType											m_type;

			VkDestroyer(VkImage)                m_image;
			VkDestroyer(VkDeviceMemory)         m_imageMemory;
			VkDestroyer(VkImageView)            m_imageView;
		};


		class StorageImage{
		public:

			StorageImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat f);

			void allocate();

			virtual VkImageLayout getLayout();

			VkImageView getImageView();

		private:

			uint32_t														m_width  = 0;
			uint32_t														m_height = 0;
			uint32_t														m_depth  = 0;
			VkFormat														m_format;

			VkDestroyer(VkImage)                m_image;
			VkDestroyer(VkDeviceMemory)         m_imageMemory;
			VkDestroyer(VkImageView)            m_imageView;

		};
	}
}