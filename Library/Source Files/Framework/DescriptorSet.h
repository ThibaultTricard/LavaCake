
#include "UniformBuffer.h"
#include "Texture.h"
#include "Constant.h"
#ifdef RAYTRACING
#include "RayTracing/TopLevelAS.h"
#endif
namespace LavaCake {
  namespace Framework{
  struct uniform {
    UniformBuffer*          buffer;
    int                     binding;
    VkShaderStageFlags      stage;
  };
  
  struct texture {
    Image*                  i;
    int                     binding;
    VkShaderStageFlags      stage;
  };
  
  struct frameBuffer {
    FrameBuffer*            f;
    int                     binding;
    VkShaderStageFlags      stage;
    uint32_t                viewIndex;
  };
  
  struct attachment {
    Image*                  i;
    int                     binding;
    VkShaderStageFlags      stage;
  };
  
  struct storageImage {
    Image*                  i;
    int                     binding;
    VkShaderStageFlags      stage;
  };
  
  struct texelBuffer {
    Buffer*                 t;
    int                     binding;
    VkShaderStageFlags      stage;
  };
  struct buffer {
    Buffer*                 t;
    int                     binding;
    VkShaderStageFlags      stage;
  };

#ifdef RAYTRACING
#include "RayTracing/TopLevelAS.h"

  struct accelerationStructure {
    LavaCake::RayTracing::TopLevelAccelerationStructure* AS;
    uint32_t								binding;
    VkShaderStageFlags			stage;
  };
#endif
  
  class DescriptorSet{
   
    public :
    
    DescriptorSet(){};
    
    ~DescriptorSet(){
      Device* d = Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      
      if (VK_NULL_HANDLE != m_descriptorPool) {
        vkDestroyDescriptorPool(logical, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
      }
      
      if (VK_NULL_HANDLE != m_descriptorSetLayout) {
        vkDestroyDescriptorSetLayout(logical, m_descriptorSetLayout, nullptr);
        m_descriptorSetLayout = VK_NULL_HANDLE;
      }
    }
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
    virtual void addTextureBuffer(Image* texture, VkShaderStageFlags stage, int binding = 0) {
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
    virtual void addStorageImage(Image* storage, VkShaderStageFlags stage, int binding = 0) {
      m_storageImages.push_back({ storage,binding,stage });
    };
    
    
    /**
     \brief Add an attachment to the pipeline and scpecify it's binding and shader stage
     \param attachement a pointer to the attachement
     \param stage the shader stage where the storage image is going to be used
     \param binding the binding point of the storage image, 0 by default
     */
    virtual void addAttachment(Image* attachement, VkShaderStageFlags stage, int binding = 0) {
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
    
#ifdef RAYTRACING
    void addAccelerationStructure(LavaCake::RayTracing::TopLevelAccelerationStructure* AS, VkShaderStageFlags stage, uint32_t	binding) {
      m_AS.push_back({ AS, binding, stage });
    }
#endif

    std::vector<attachment>& getAttachments() {
      return m_attachments;
    };
    
    void generateDescriptorLayout() {
      
      Device* d = Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      
      std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding = {};
      
      for (uint32_t i = 0; i < m_uniforms.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_uniforms[i].binding),
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          1,
          m_uniforms[i].stage,
          nullptr
        });
      }
      for (uint32_t i = 0; i < m_textures.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_textures[i].binding),
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          1,
          m_textures[i].stage,
          nullptr
        });
      }
      for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_frameBuffers[i].binding),
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          1,
          m_frameBuffers[i].stage,
          nullptr
        });
      }
      for (uint32_t i = 0; i < m_attachments.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_attachments[i].binding),
          VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
          1,
          m_attachments[i].stage,
          nullptr
        });
      }
      for (uint32_t i = 0; i < m_storageImages.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_storageImages[i].binding),
          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          1,
          m_storageImages[i].stage,
          nullptr
        });
      }
      for (uint32_t i = 0; i < m_texelBuffers.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_texelBuffers[i].binding),
          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
          1,
          m_texelBuffers[i].stage,
          nullptr
        });
      }
      for (uint32_t i = 0; i < m_buffers.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_buffers[i].binding),
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          1,
          m_buffers[i].stage,
          nullptr
          });
      }
#ifdef RAYTRACING
      for (uint32_t i = 0; i < m_AS.size(); i++) {
        descriptorSetLayoutBinding.push_back({
          uint32_t(m_AS[i].binding),
          VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
          1,
          m_AS[i].stage,
          nullptr
          });
      }
#endif
      
      if (!Core::CreateDescriptorSetLayout(logical, descriptorSetLayoutBinding, m_descriptorSetLayout)) {
        ErrorCheck::setError((char*)"Can't create descriptor set layout");
      }
      
      uint32_t descriptorsNumber = static_cast<uint32_t>(m_uniforms.size() + m_textures.size() + m_storageImages.size() + m_attachments.size() + m_frameBuffers.size() + m_texelBuffers.size() + m_buffers.size());
      
#ifdef RAYTRACING
      descriptorsNumber += m_AS.size();
#endif
      if (descriptorsNumber == 0) {
        m_empty = true;
        return;
      }
      else {
        m_empty = false;
      }
      
      std::vector<VkDescriptorPoolSize>  descriptorPoolSize = {};
      
      if (m_uniforms.size() > 0) {
        descriptorPoolSize.push_back({
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          uint32_t(m_uniforms.size())
        });
      }
      if (m_textures.size() + m_frameBuffers.size() > 0) {
        descriptorPoolSize.push_back({
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          uint32_t(m_textures.size() + m_frameBuffers.size())
        });
      }
      if (m_attachments.size() > 0) {
        descriptorPoolSize.push_back({
          VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
          uint32_t(m_attachments.size())
        });
      }
      if (m_storageImages.size() > 0) {
        descriptorPoolSize.push_back({
          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          uint32_t(m_storageImages.size())
        });
      }
      if (m_texelBuffers.size() > 0) {
        descriptorPoolSize.push_back({
          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
          uint32_t(m_texelBuffers.size())
        });
      }
      if (m_buffers.size() > 0) {
        descriptorPoolSize.push_back({
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            uint32_t(m_buffers.size())
          });
      }

#ifdef RAYTRACING
      if (m_AS.size() > 0) {
        descriptorPoolSize.push_back({
          VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
            uint32_t(m_AS.size())
          });
      }
#endif // RAYTRACING

      
      
      if (!Core::CreateDescriptorPool(logical, false, descriptorsNumber, descriptorPoolSize, m_descriptorPool)) {
        ErrorCheck::setError((char*)"Can't create descriptor pool");
      }
      
      std::vector<VkDescriptorSet> descriptorSets;
      if (!Core::AllocateDescriptorSets(logical, m_descriptorPool, { m_descriptorSetLayout }, descriptorSets)) {
        ErrorCheck::setError((char*)"Can't allocate descriptor set");
      }
      std::vector<Core::BufferDescriptorInfo> bufferDescriptorUpdate = { };
      int descriptorCount = 0;
      for (uint32_t i = 0; i < m_uniforms.size(); i++) {
        bufferDescriptorUpdate.push_back({
          descriptorSets[descriptorCount],              // VkDescriptorSet                      TargetDescriptorSet
          uint32_t(m_uniforms[i].binding),                // uint32_t                             TargetDescriptorBinding
          0,                                              // uint32_t                             TargetArrayElement
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,              // VkDescriptorType                     TargetDescriptorType
          {                                                // std::vector<VkDescriptorBufferInfo>  BufferInfos
            {
            m_uniforms[i].buffer->getHandle(),          // VkBuffer                             buffer
            0,                                          // VkDeviceSize                         offset
            VK_WHOLE_SIZE                                // VkDeviceSize                         range
            }
          }
        });
      }
      
      std::vector<Core::ImageDescriptorInfo> imageDescriptorUpdate = { };
      for (uint32_t i = 0; i < m_textures.size(); i++) {
        imageDescriptorUpdate.push_back({
          descriptorSets[descriptorCount],              // VkDescriptorSet                      TargetDescriptorSet
          uint32_t(m_textures[i].binding),                // uint32_t                             TargetDescriptorBinding
          0,                                              // uint32_t                             TargetArrayElement
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType                     TargetDescriptorType
          {                                                // std::vector<VkDescriptorBufferInfo>  BufferInfos
            {
            m_textures[i].i->getSampler(),              // vkSampler                            buffer
            m_textures[i].i->getImageView(),            // VkImageView                          offset
            m_textures[i].i->getLayout()                // VkImageLayout                         range
            }
          }
        });
      }
      for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
        std::vector<VkDescriptorImageInfo> descriptorInfo;
        descriptorInfo.push_back(
                                 {
                                 m_frameBuffers[i].f->getSampler(),
                                 m_frameBuffers[i].f->getImageViews(m_frameBuffers[i].viewIndex),
                                 m_frameBuffers[i].f->getLayout(m_frameBuffers[i].viewIndex),
                                 });
        
        imageDescriptorUpdate.push_back({
          descriptorSets[descriptorCount],              // VkDescriptorSet                      TargetDescriptorSet
          uint32_t(m_frameBuffers[i].binding),            // uint32_t                             TargetDescriptorBinding
          0,                                              // uint32_t                             TargetArrayElement
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType                     TargetDescriptorType
          descriptorInfo                                  // std::vector<VkDescriptorBufferInfo>  BufferInfos
        });
      }
      for (uint32_t i = 0; i < m_attachments.size(); i++) {
        imageDescriptorUpdate.push_back({
          descriptorSets[descriptorCount],              // VkDescriptorSet                      TargetDescriptorSet
          uint32_t(m_attachments[i].binding),                // uint32_t                             TargetDescriptorBinding
          0,                                              // uint32_t                             TargetArrayElement
          VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,            // VkDescriptorType                     TargetDescriptorType
          {                                                // std::vector<VkDescriptorBufferInfo>  BufferInfos
            {
            VK_NULL_HANDLE,                              // vkSampler                            buffer
            m_attachments[i].i->getImageView(),          // VkImageView                          offset
            m_attachments[i].i->getLayout()              // VkImageLayout                         range
            }
          }
        });
      }
      
      for (uint32_t i = 0; i < m_storageImages.size(); i++) {
        imageDescriptorUpdate.push_back({
          descriptorSets[descriptorCount],              // VkDescriptorSet                      TargetDescriptorSet
          uint32_t(m_storageImages[i].binding),            // uint32_t                             TargetDescriptorBinding
          0,                                              // uint32_t                             TargetArrayElement
          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                // VkDescriptorType                     TargetDescriptorType
          {                                                // std::vector<VkDescriptorBufferInfo>  BufferInfos
            {
            VK_NULL_HANDLE,                              // vkSampler                            buffer
            m_storageImages[i].i->getImageView(),        // VkImageView                          offset
            m_storageImages[i].i->getLayout()            // VkImageLayout                         range
            }
          }
        });
      }
      
      std::vector<Core::TexelBufferDescriptorInfo> texelBufferDescriptorUpdate = {};
      for (uint32_t i = 0; i < m_texelBuffers.size(); i++) {
        
        Core::TexelBufferDescriptorInfo info = {
          descriptorSets[descriptorCount],
          uint32_t(m_texelBuffers[i].binding),
          0,
          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
          {
          m_texelBuffers[i].t->getBufferView()
          }
        };
        
        texelBufferDescriptorUpdate.push_back(info);
      }

      for (uint32_t i = 0; i < m_buffers.size(); i++) {

        LavaCake::Core::BufferDescriptorInfo info = {
          descriptorSets[descriptorCount],
          uint32_t(m_buffers[i].binding),
          0,
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          {
            {
              m_buffers[i].t->getHandle(),
              0,
              VK_WHOLE_SIZE
            }
          }
        };

        bufferDescriptorUpdate.push_back(info);
      }

      /////////////////////////////////////////////////////////////////////////////////////////////
      std::vector<VkWriteDescriptorSet> write_descriptors;
      std::vector<VkCopyDescriptorSet> copy_descriptors;

      // image descriptors
      for (auto& image_descriptor : imageDescriptorUpdate) {
        write_descriptors.push_back({
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
          nullptr,                                                                // const void                     * pNext
          image_descriptor.TargetDescriptorSet,                                   // VkDescriptorSet                  dstSet
          image_descriptor.TargetDescriptorBinding,                               // uint32_t                         dstBinding
          image_descriptor.TargetArrayElement,                                    // uint32_t                         dstArrayElement
          static_cast<uint32_t>(image_descriptor.ImageInfos.size()),              // uint32_t                         descriptorCount
          image_descriptor.TargetDescriptorType,                                  // VkDescriptorType                 descriptorType
          image_descriptor.ImageInfos.data(),                                     // const VkDescriptorImageInfo    * pImageInfo
          nullptr,                                                                // const VkDescriptorBufferInfo   * pBufferInfo
          nullptr                                                                 // const VkBufferView             * pTexelBufferView
          });
      }

      // buffer descriptors
      for (auto& buffer_descriptor : bufferDescriptorUpdate) {
        write_descriptors.push_back({
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
          nullptr,                                                                // const void                     * pNext
          buffer_descriptor.TargetDescriptorSet,                                  // VkDescriptorSet                  dstSet
          buffer_descriptor.TargetDescriptorBinding,                              // uint32_t                         dstBinding
          buffer_descriptor.TargetArrayElement,                                   // uint32_t                         dstArrayElement
          static_cast<uint32_t>(buffer_descriptor.BufferInfos.size()),            // uint32_t                         descriptorCount
          buffer_descriptor.TargetDescriptorType,                                 // VkDescriptorType                 descriptorType
          nullptr,                                                                // const VkDescriptorImageInfo    * pImageInfo
          buffer_descriptor.BufferInfos.data(),                                   // const VkDescriptorBufferInfo   * pBufferInfo
          nullptr                                                                 // const VkBufferView             * pTexelBufferView
          });
      }

      // texel buffer descriptors
      for (auto& texel_buffer_descriptor : texelBufferDescriptorUpdate) {
        write_descriptors.push_back({
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
          nullptr,                                                                // const void                     * pNext
          texel_buffer_descriptor.TargetDescriptorSet,                            // VkDescriptorSet                  dstSet
          texel_buffer_descriptor.TargetDescriptorBinding,                        // uint32_t                         dstBinding
          texel_buffer_descriptor.TargetArrayElement,                             // uint32_t                         dstArrayElement
          static_cast<uint32_t>(texel_buffer_descriptor.TexelBufferViews.size()), // uint32_t                         descriptorCount
          texel_buffer_descriptor.TargetDescriptorType,                           // VkDescriptorType                 descriptorType
          nullptr,                                                                // const VkDescriptorImageInfo    * pImageInfo
          nullptr,                                                                // const VkDescriptorBufferInfo   * pBufferInfo
          texel_buffer_descriptor.TexelBufferViews.data()                         // const VkBufferView             * pTexelBufferView
          });
      }


      std::vector<VkWriteDescriptorSetAccelerationStructureKHR> descriptorAccelerationStructureInfos{};
#ifdef RAYTRACING
      for (auto& AS_descriptor : m_AS) {
        descriptorAccelerationStructureInfos.push_back({
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
          nullptr,
          1,
          &AS_descriptor.AS->getHandle()
          });
      }


      // acceleration structure descriptors
      for (int i = 0; i < m_AS.size(); i++) {
        write_descriptors.push_back({
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
          &descriptorAccelerationStructureInfos[i],																																// const void                     * pNext
          descriptorSets[descriptorCount],																			// VkDescriptorSet                  dstSet
          m_AS[i].binding,																												// uint32_t                         dstBinding
          0,																																			// uint32_t                         dstArrayElement
          static_cast<uint32_t>(1),																								// uint32_t                         descriptorCount
          VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,                          // VkDescriptorType                 descriptorType
          nullptr,                                                                // const VkDescriptorImageInfo    * pImageInfo
          nullptr,                                                                // const VkDescriptorBufferInfo   * pBufferInfo
          {	}
          });
      }
#endif

      vkUpdateDescriptorSets(logical, static_cast<uint32_t>(write_descriptors.size()), write_descriptors.data(), static_cast<uint32_t>(copy_descriptors.size()), copy_descriptors.data());
      
      m_descriptorSet =descriptorSets[0];
    }
    
    
    VkDescriptorSet& getHandle(){
      return m_descriptorSet;
    }
    
    VkDescriptorSetLayout& getLayout(){
      return m_descriptorSetLayout;
    }
    
    bool isEmpty(){
      return m_empty;
    }
    
    private :

    VkDescriptorSet                                                 m_descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout                                           m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool                                                m_descriptorPool = VK_NULL_HANDLE;
    
    std::vector<uniform>                                            m_uniforms;
    std::vector<texture>                                            m_textures;
    std::vector<frameBuffer>                                        m_frameBuffers;
    std::vector<attachment>                                         m_attachments;
    std::vector<storageImage>                                       m_storageImages;
    std::vector<texelBuffer>                                        m_texelBuffers;
    std::vector<buffer>                                             m_buffers;

#ifdef RAYTRACING
    std::vector<accelerationStructure>                              m_AS;
#endif
    bool                                                            m_empty = true;
  };
  }
}