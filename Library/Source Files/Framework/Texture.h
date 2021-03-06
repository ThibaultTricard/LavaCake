#pragma once

#include "AllHeaders.h"
#include "VulkanDestroyer.h"
#include "Queue.h"
#include "Device.h"
#include "ErrorCheck.h"
#include "SwapChain.h"
#include "CommandBuffer.h"
#include "Image.h"
#include "Math/basics.h"

namespace LavaCake {
	namespace Framework {
  /**
   Class TextureBuffer
   \brief This class wrap and configure an Image to be used as 2D texture buffer
   */
		class TextureBuffer {
		public:


			/**
			\brief construct a texture buffer from a file
			\param filename : the name of the file where the data is stored.
			\param int nbChannel : the number of channel that contain the texture between 1 and 4.
			\param VkFormat f : the format of theTexture buffer.
			*/
			TextureBuffer(std::string filename, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);


			/**
			\brief construct a texture buffer from a array of data.
			\param data : the data that will be used of the texture.
			\param width : the width of the texture.
			\param height : the height of the texture.
			\param nbChannel : the number of channel that contain the texture between 1 and 4.
			\param f : the format of theTexture buffer.
			*/
			TextureBuffer(std::vector<unsigned char>* data, int width, int height, int nbChannel, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);


			/**
			\brief allocate the Texture buffer on the device on the GPU
      \param queue : a pointer to the queue that will be used to copy data to the Buffer
      \param cmdBuff : the command buffer used for this operation, must not be in a recording state
      \param stageFlagBit : the stage where the buffer will be used, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html">here</a>
			*/
			virtual void allocate(Queue* queue, CommandBuffer& cmdBuff, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

			/**
      \brief get the sampler of the texture buffer
			\return a VkSampler
			*/
			VkSampler&	getSampler();

			/**
      \brief get the image view of the texture buffer
			\return a VkImageView
			*/
			VkImageView& getImageView();

			/**
      \brief get the image layout of the texture buffer
			\return a VkImageLayout
			*/
			virtual VkImageLayout getLayout();

      /**
       \brief get the width of the texture
       \return a uint32_t
       */
			uint32_t width() {
				return m_image->width();
			}
      
      /**
       \brief get the height of the texture
       \return a uint32_t
       */
			uint32_t height() {
				return m_image->height();
			}

			~TextureBuffer() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				if (VK_NULL_HANDLE != *m_sampler) {
					vkDestroySampler(logical, *m_sampler, nullptr);
					*m_sampler = VK_NULL_HANDLE;
				}

				delete(m_image);
			}

		protected :

			TextureBuffer() {};

			Image*																		m_image = nullptr;
			uint32_t																	m_nbChannel = 0;
			std::vector<unsigned char>*								m_data = new std::vector<unsigned char>();


			
			VkDestroyer(VkSampler)										m_sampler;


			VkFormat																	m_format = VK_FORMAT_UNDEFINED;
		};

    /**
     Class TextureBuffer3D
     \brief This class wrap and configure an Image to be used as 3D texture buffer
     */
		class TextureBuffer3D : public TextureBuffer {
		public:

			/**
			\brief construct a texture buffer from a array of data.
			\param data : the data that will be used of the texture.
			\param int width : the width of the texture.
			\param int height : the height of the texture.
			\param int depth : the depth of the texture.
			\param int nbChannel : the number of channel that contain the texture between 1 and 4.
			\param VkFormat f : the format of theTexture buffer.
			*/
			TextureBuffer3D(std::vector<unsigned char>* data, int width, int height, int depth, int nbChannel, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

      /**
       \brief allocate the Texture buffer 3D on the device on the GPU
       \param queue : a pointer to the queue that will be used to copy data to the Buffer
       \param cmdBuff : the command buffer used for this operation, must not be in a recording state
       \param stageFlagBit : the stage where the buffer will be used, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html">here</a>
       */
			virtual void allocate(Queue* queue, CommandBuffer& cmdBuff, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		private:

			uint32_t																	m_depth = 1;
			
		};

    /**
    Class TextureBuffer
    \brief This class wrap and configure an Image to be used as a cubeMap
    */
		class CubeMap : public TextureBuffer {
		public:

			/**
			\brief create a CubeMap from 6 textures file.
			\param path : path to a folder where the 6 texture file are
			\param int nbChannel : the number of channel that contain the texture between 1 and 4.
			\param std::vector<std::string> images : a list of six name of texture in the folder
			\param VkFormat f : the format of theTexture buffer.
			*/
			CubeMap(std::string path, int nbChannel, std::vector<std::string> images = { "posx.jpg","negx.jpg","posy.jpg","negy.jpg","posz.jpg","negz.jpg" }, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);

			/**
       \brief Allocate the CubeMap on the GPU
       \param queue : a pointer to the queue that will be used to copy data to the Buffer
       \param cmdBuff : the command buffer used for this operation, must not be in a recording state
       \param stageFlagBit : the stage where the buffer will be used, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html">here</a>
			*/
			virtual void allocate(Queue* queue, CommandBuffer& cmdBuff, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT) override;

		private:
			std::string													m_path;
			std::vector<std::string>						m_images;
		};

    /**
     Class FrameBuffer
     \brief a class to help manage frame buffers
     */
		class FrameBuffer {
		public : 
			
			/**
			\brief create a FrameBuffer.
			\param width						: the width of the FrameBuffer.
			\param height						: the height of the FrameBuffer.
			*/
			FrameBuffer(uint32_t width, uint32_t height);

			/**
			\brief get the image layout of one layer of the FrameBuffer
      \param i the index of the layer
      \return a VkImageLayout
			*/
			VkImageLayout& getLayout(uint8_t i);


			/**
			\brief get the sampler of the frameBuffer
      \return a VkSampler
			*/
			VkSampler&	getSampler();

			/**
			\brief get the image view of one layer of the Framebuffer
      \param i the index of the layer
      \brief a VkImageView
			*/
			VkImageView&	 getImageViews(uint8_t i);

			/**
			\brief get the number of image view in the Framebuffer
      \return a size_t
			*/
			size_t getImageViewSize();

			/**
			\brief get the handle of the Framebuffer
      \return a VkFramebuffer
			*/
			VkFramebuffer& getHandle();

      /**
      \brief get the resolution of the Framebuffer
      \return a vec2u
      */
			vec2u size() {
				return vec2u({ m_width , m_height });
			}

			~FrameBuffer() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				

				for (uint32_t i = 0; i < m_images.size(); i++) {
					if (i != m_swapChainImageIndex) {
						if (VK_NULL_HANDLE != m_images[i]) {
							vkDestroyImage(logical, m_images[i], nullptr);
							m_images[i] = VK_NULL_HANDLE;
						}

						if (VK_NULL_HANDLE != m_imageViews[i]) {
							vkDestroyImageView(logical, m_imageViews[i], nullptr);
							m_imageViews[i] = VK_NULL_HANDLE;
						}
					}
				}

				if (VK_NULL_HANDLE != *m_sampler) {
					vkDestroySampler(logical, *m_sampler, nullptr);
					*m_sampler = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_frameBuffer) {
					vkDestroyFramebuffer(logical, *m_frameBuffer, nullptr);
					*m_frameBuffer = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_imageMemory) {
					vkFreeMemory(logical, *m_imageMemory, nullptr);
					*m_imageMemory = VK_NULL_HANDLE;
				}
			}

		private :

			uint32_t																								m_width = 0;
			uint32_t																								m_height = 0;

			VkDestroyer(VkFramebuffer)															m_frameBuffer;
			VkDestroyer(VkSampler)																	m_sampler;
			VkDestroyer(VkDeviceMemory)															m_imageMemory;
			std::vector<VkImage>																		m_images;
			std::vector<VkImageView>																m_imageViews;
			std::vector<VkImageLayout>															m_layouts;

			uint32_t																								m_swapChainImageIndex;
			friend class RenderPass;
		};

		enum attachmentType {
			COLOR_ATTACHMENT ,DEPTH_ATTACHMENT, STENCIL_ATTACHMENT
		};

		class Attachment {
		public :

			Attachment(int width, int height, VkFormat f, attachmentType type);

			void allocate();

			virtual VkImageLayout getLayout();

			VkImageView getImageView();

			VkImage getImage();

			~Attachment() {
				delete(m_image);
			}

		private : 

			Image*															m_image;
			attachmentType											m_type;

		};


		class StorageImage{
		public:

			StorageImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat f);

			void allocate(Queue* queue, CommandBuffer& cmdBuff);

			virtual VkImageLayout getLayout();

			VkImageView getImageView();

			~StorageImage() {
				delete(m_image);
			}


		private:

			Image* m_image;

		};

	}
}
