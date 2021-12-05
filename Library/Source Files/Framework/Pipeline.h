#pragma once

#include "ShaderModule.h"
#include "VertexBuffer.h"
#include "UniformBuffer.h"
#include "Texture.h"
#include "Constant.h"

namespace LavaCake {
	namespace Framework {

		struct uniform {
			UniformBuffer*          buffer;
			int											binding;
			VkShaderStageFlags			stage;
		};

		struct texture {
			TextureBuffer*          t;
			int                     binding;
			VkShaderStageFlags			stage;
		};

		struct frameBuffer {
			FrameBuffer*            f;
			int                     binding;
			VkShaderStageFlags			stage;
			uint32_t                viewIndex;
		};

		struct attachment {
			Attachment*             a;
			int                     binding;
			VkShaderStageFlags			stage;
		};

		struct storageImage {
			StorageImage*           s;
			int                     binding;
			VkShaderStageFlags			stage;
		};

		struct constant {
			PushConstant*           constant;
			VkShaderStageFlags			stage;
		};

		struct texelBuffer {
			Buffer*                 t;
			int                     binding;
			VkShaderStageFlags			stage;
		};
		struct buffer {
			Buffer*                 t;
			int                     binding;
			VkShaderStageFlags			stage;
		};

    /**
     Class Pipeline :
     \brief A generic class to help manage VkPipelines, inherited by the classes ComputePipeline and GraphicPipeline
     */
		class Pipeline {
		public :
			/**
       \brief Add a uniform Buffer to the pipeline and scpecify it's binding and shader stage
       \param uniform a pointer to the uniform buffer
       \param stage the shader stage where the uniform buffer is going to be used
       \param binding the binding point of the uniform shader, 0 by default
      */
			virtual void addUniformBuffer(UniformBuffer* uniform, VkShaderStageFlags stage, int binding = 0) {
				m_uniforms.push_back({uniform ,binding,stage});
			};

			/**
			 \brief Add a texture Buffer to the pipeline and scpecify it's binding and shader stage
       \param texture a pointer to the texture buffer
       \param stage the shader stage where the texture buffer is going to be used
       \param binding the binding point of the texture buffer, 0 by default
			*/
			virtual void addTextureBuffer(TextureBuffer* texture, VkShaderStageFlags stage, int binding = 0) {
				m_textures.push_back({ texture,binding,stage});
			};

			/**
			 \brief Add a frame Buffer to the pipeline and scpecify it's binding and shader stage
       \param frame a pointer to the frame buffer
       \param stage the shader stage where the frame buffer is going to be used
       \param binding the binding point of the frame buffer, 0 by default
			*/
			virtual void addFrameBuffer(FrameBuffer* frame, VkShaderStageFlags stage, int binding = 0, uint32_t view = 0) {
				m_frameBuffers.push_back({frame,binding,stage,view});
			}; 

			/**
			 \brief Add a storage Image to the pipeline and scpecify it's binding and shader stage
       \param storage a pointer to the frame buffer
       \param stage the shader stage where the storage image is going to be used
       \param binding the binding point of the storage image, 0 by default
			*/
			virtual void addStorageImage(StorageImage* storage, VkShaderStageFlags stage, int binding = 0) {
				m_storageImages.push_back({ storage,binding,stage });
			}; 


			/**
			 \brief Add an attachment to the pipeline and scpecify it's binding and shader stage
       \param attachement a pointer to the attachement
       \param stage the shader stage where the storage image is going to be used
       \param binding the binding point of the storage image, 0 by default
			*/
			virtual void addAttachment(Attachment* attachement, VkShaderStageFlags stage, int binding = 0) {
				m_attachments.push_back({ attachement,binding,stage });
			};

      /**
       \brief Add a texel buffer to the pipeline and scpecify it's binding and shader stage
       \param texel a pointer to the texel buffer
       \param stage the shader stage where the texel buffer is going to be used
       \param binding the binding point of the texel buffer, 0 by default
       */
			virtual void addTexelBuffer(Buffer* texel, VkShaderStageFlags stage, int binding = 0) {
				m_texelBuffers.push_back({ texel,binding,stage });
			};
      
      /**
       \brief Add a buffer to the pipeline and scpecify it's binding and shader stage
       \param buffer a pointer to the texel buffer
       \param stage the shader stage where the texel buffer is going to be used
       \param binding the binding point of the texel buffer, 0 by default
       */
			virtual void addBuffer(Buffer* buffer, VkShaderStageFlags stage, int binding = 0) {
				m_buffers.push_back({ buffer,binding,stage });
			};

      
			std::vector<attachment>& getAttachments() {
				return m_attachments;
			};

			~Pipeline() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				if (VK_NULL_HANDLE != m_pipeline) {
					vkDestroyPipeline(logical, m_pipeline, nullptr);
					m_pipeline = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_pipelineLayout) {
					vkDestroyPipelineLayout(logical, m_pipelineLayout, nullptr);
					m_pipelineLayout = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_descriptorPool) {
					vkDestroyDescriptorPool(logical, m_descriptorPool, nullptr);
					m_descriptorPool = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_descriptorSetLayout) {
					vkDestroyDescriptorSetLayout(logical, m_descriptorSetLayout, nullptr);
					m_descriptorSetLayout = VK_NULL_HANDLE;
				}
			};

		protected :
			virtual void generateDescriptorLayout();

			void SpecifyPipelineShaderStages(std::vector<Framework::ShaderStageParameters> const& shader_stage_params,
				std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_create_infos);


			bool CreatePipelineLayout(VkDevice                                   logical_device,
				std::vector<VkDescriptorSetLayout> const& descriptor_set_layouts,
				std::vector<VkPushConstantRange> const& push_constant_ranges,
				VkPipelineLayout& pipeline_layout);

			bool CreateGraphicsPipelines(VkDevice                                             logical_device,
				std::vector<VkGraphicsPipelineCreateInfo> const& graphics_pipeline_create_infos,
				VkPipelineCache                                      pipeline_cache,
				std::vector<VkPipeline>& graphics_pipelines);




			VkPipeline																					            m_pipeline = VK_NULL_HANDLE;
			VkPipelineLayout																		            m_pipelineLayout = VK_NULL_HANDLE;

			VkDescriptorSetLayout															              m_descriptorSetLayout = VK_NULL_HANDLE;
			VkDescriptorPool																		            m_descriptorPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet>																		m_descriptorSets;
			std::vector<VkDescriptorPoolSize>																m_descriptorPoolSize;
			std::vector<VkDescriptorSetLayoutBinding>												m_descriptorSetLayoutBinding;
			uint32_t																												m_descriptorCount = 0;

			std::vector<Core::BufferDescriptorInfo>													m_bufferDescriptorUpdate;
			std::vector<Core::ImageDescriptorInfo>													m_imageDescriptorUpdate;
			std::vector<Core::TexelBufferDescriptorInfo>										m_texelBufferDescriptorUpdate;

			std::vector<uniform>																						m_uniforms;
			std::vector<texture>																						m_textures;
			std::vector<frameBuffer>																				m_frameBuffers;
			std::vector<attachment>																					m_attachments;
			std::vector<storageImage>																				m_storageImages;
			std::vector<texelBuffer>																				m_texelBuffers;
			std::vector<buffer>																							m_buffers;
			std::vector<constant>																						m_constants;
		};
	}
}
