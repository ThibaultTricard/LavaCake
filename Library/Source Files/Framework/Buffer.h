#pragma once

#include "CommandBuffer.h"
#include "Queue.h"
#include "Image.h"

namespace LavaCake {
  namespace Framework {
    
		class Image;
  
  /**
		Class Buffer :
		\brief This class helps manage Vulkan Buffers their memory and view
  */
    class Buffer {
    public :
      
      /**
				\brief Default constructor
      */
			Buffer();

      
			Buffer(const Buffer& buffer) = delete;

			Image& operator=(const Image&) = delete;


			Buffer(Buffer&& b) {
				m_buffer = b.m_buffer;
				m_bufferMemory = b.m_bufferMemory;
				m_bufferView = b.m_bufferView;


				m_stage = b.m_stage;
				m_access = b.m_access;
				m_queueFamily = b.m_queueFamily;
				m_dataSize = b.m_dataSize;
				m_padding = b.m_padding;

				m_mapped = b.m_mapped;

				b.m_buffer = VK_NULL_HANDLE;
				b.m_bufferMemory = VK_NULL_HANDLE;
				b.m_bufferView = VK_NULL_HANDLE;
				b.m_mapped = nullptr;
			}
      
      
			/**
				\brief Allocate and initialise the Buffer with a list of data
				\param queue : a pointer to the queue that will be used to copy data to the Buffer
				\param cmdBuff : the command buffer used for this operation, must not be in a recording state
				\param rawdata : a vector of data to be pushed to the buffer
				\param usage : the usage of the buffer see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkBufferUsageFlags.html">here</a>
				\param memPropertyFlag : the memory property of the buffer, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryPropertyFlagBits.html">here</a>
				\param stageFlagBit : the stage where the buffer will be used, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html">here</a>
				\param format : the format of the buffer  see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFormat.html">here</a>
				\param accessmod : the access mode of the buffer <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkAccessFlagBits.html">here</a>
      */
			template <typename t>
			void allocate(Queue& queue, CommandBuffer& cmdBuff, const std::vector<t>& rawdata, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_TRANSFER_BIT, VkFormat format = VK_FORMAT_R32_SFLOAT, VkAccessFlagBits accessmod = VK_ACCESS_TRANSFER_WRITE_BIT) {
				Device* d = Device::getDevice();
				VkPhysicalDevice physical = d->getPhysicalDevice();
				VkDevice logical = d->getLogicalDevice();
				m_dataSize = uint64_t(rawdata.size() * sizeof(t));
				
				VkPhysicalDeviceProperties p;
				vkGetPhysicalDeviceProperties(physical,&p);

				m_padding = p.limits.nonCoherentAtomSize - m_dataSize % p.limits.nonCoherentAtomSize;



				m_stage = stageFlagBit;
				m_access = accessmod;
				m_queueFamily = queue.getIndex();

				uint32_t memProp = 0;
				if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
					memProp = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
				}

				if (VK_NULL_HANDLE != m_buffer) {
					vkDestroyBuffer(logical, m_buffer, nullptr);
					m_buffer = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_bufferView) {
					vkDestroyBufferView(logical, m_bufferView, nullptr);
					m_bufferView = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_bufferMemory) {
					vkFreeMemory(logical, m_bufferMemory, nullptr);
					m_bufferMemory = VK_NULL_HANDLE;
				}


				VkBufferCreateInfo buffer_create_info = {
				VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType
				nullptr,                                // const void           * pNext
				0,                                      // VkBufferCreateFlags    flags
				m_dataSize + m_padding,                 // VkDeviceSize           size
				VK_BUFFER_USAGE_TRANSFER_DST_BIT| usage,// VkBufferUsageFlags     usage
				VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode
				0,                                      // uint32_t               queueFamilyIndexCount
				nullptr                                 // const uint32_t       * pQueueFamilyIndices
				};

				VkResult result = vkCreateBuffer(logical, &buffer_create_info, nullptr, &m_buffer);

				if (result != VK_SUCCESS) {
					ErrorCheck::setError("Can't create Buffer");
				}


				VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
				vkGetPhysicalDeviceMemoryProperties(physical, &physical_device_memory_properties);

				VkMemoryRequirements memory_requirements;
				vkGetBufferMemoryRequirements(logical, m_buffer, &memory_requirements);

				m_bufferMemory = VK_NULL_HANDLE;
				for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type) {
					if ((memory_requirements.memoryTypeBits & (1 << type)) &&
						((physical_device_memory_properties.memoryTypes[type].propertyFlags & memPropertyFlag) == memPropertyFlag)) {

						VkMemoryAllocateFlagsInfo next{
						VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
						nullptr,
						memProp,
            0
						};

						VkMemoryAllocateInfo buffer_memory_allocate_info = {
							VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
							&next,                                  // const void       * pNext
							memory_requirements.size,                 // VkDeviceSize       allocationSize
							type                          // uint32_t           memoryTypeIndex
						};

						result = vkAllocateMemory(logical, &buffer_memory_allocate_info, nullptr, &m_bufferMemory);
						if (VK_SUCCESS == result) {
							break;
						}
					}
				}

				if (VK_NULL_HANDLE == m_bufferMemory) {
					ErrorCheck::setError("Could not allocate memory for a buffer.");
				}

				result = vkBindBufferMemory(logical, m_buffer, m_bufferMemory, 0);
				if (VK_SUCCESS != result) {
					ErrorCheck::setError("Could not bind memory object to a buffer.");
				}

				if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
					VkBufferViewCreateInfo buffer_view_create_info = {
					VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,    // VkStructureType            sType
					nullptr,                                      // const void               * pNext
					0,                                            // VkBufferViewCreateFlags    flags
					m_buffer,                                     // VkBuffer                   buffer
					format,                                       // VkFormat                   format
					0,																						// VkDeviceSize               offset
					VK_WHOLE_SIZE                                 // VkDeviceSize               range
					};

					result = vkCreateBufferView(logical, &buffer_view_create_info, nullptr, &m_bufferView);
					if (VK_SUCCESS != result) {
						ErrorCheck::setError("Could not creat buffer view.");
					}
				}

				Buffer stagingBuffer;

				stagingBuffer.allocate(m_dataSize + m_padding, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

				stagingBuffer.write(rawdata);

				cmdBuff.resetFence();
				cmdBuff.beginRecord();

				setAccess(cmdBuff, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED);

				stagingBuffer.copyToBuffer(cmdBuff, *this, { { 0, 0, m_dataSize + m_padding  } });

				setAccess(cmdBuff, m_stage, m_access, VK_QUEUE_FAMILY_IGNORED);


				cmdBuff.endRecord();

				cmdBuff.submit(queue, {}, {});

				cmdBuff.wait(UINT32_MAX);
				cmdBuff.resetFence();
				/*if (!LavaCake::Core::UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physical, logical, sizeof(rawdata[0]) * rawdata.size(), &rawdata[0],
					*m_buffer, 0, 0, accessmod, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, stageFlagBit, queue->getHandle(), cmdBuff.getHandle() , {})) {
					ErrorCheck::setError("Can't copy data to buffer");
				}*/
      }

      
      
      /**
				\brief Allocate a Buffer of a given size
				\param byteSize : the size in byte of the buffer
				\param usage : the usage of the buffer see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkBufferUsageFlags.html">here</a>
				\param memPropertyFlag : the memory property of the buffer, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryPropertyFlagBits.html">here</a>
				\param stageFlagBit : the stage where the buffer will be used, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html">here</a>
				\param format : the format of the buffer  see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFormat.html">here</a>
				\param accessmod : the access mode of the buffer <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkAccessFlagBits.html">here</a>
      */
			void allocate(uint64_t byteSize, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_TRANSFER_BIT, VkFormat format = VK_FORMAT_R32_SFLOAT);
      
      /**
				\brief Change the acces mode of the buffer
				\param cmdBuff : the command buffer used for this opperation, must be in a recording state
				\param dstStage : the new stage of the buffer, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html">here</a>
				\param dstAccessMode : the new access mode of the buffer <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkAccessFlagBits.html">here</a>
				\param dstQueueFamily : (optional) the new family queue of  the buffer, if ignored the buffer will remain on the same family queue
      */
			void setAccess(CommandBuffer& cmdBuff, VkPipelineStageFlags dstStage, VkAccessFlagBits dstAccessMode, uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED);

      /**
				 \brief Return the handle of the buffer
				 \return a VkBuffer: the vulkan representation of the buffer
      */
			VkBuffer& getHandle();

      /**
				 \brief Return the buffer view
				 \return a VkBuffer: the vulkan representation of the buffer
      */
			VkBufferView& getBufferView();

      /**
				 \brief Return the buffer memory
				 \return a VkDeviceMemory: the memory of the buffer on the GPU
      */
      VkDeviceMemory& getMemory();
      
      /**
				 \brief Copy a region(s) of the buffer to an image
				 \param cmdBuff: the command buffer used for this operation, must be in a  recording state
				 \param image: the destination image
				 \param regions: The listt of regions of the image to be copied to the buffer
      */
      void copyToImage(CommandBuffer& cmdBuff, Image& image, const std::vector<VkBufferImageCopy>& regions);
      
      /**
				 \brief Copy a region(s) of the buffer another buffer
				 \param cmdBuff: the command buffer used for this operation, must be in a  recording state
				 \param buffer: the destination buffer
				 \param regions: The list of regions of the image to be copied to the buffer
      */
      void copyToBuffer(CommandBuffer& cmdBuff, Buffer& buffer, const std::vector<VkBufferCopy>& regions);
      
			
      /**
				\brief Map the buffer memory to a pointer
				\return a void* pointer to mapped memory
      */
			void* map();

      /**
				\brief Unmap the buffer memory
       
      */
			void unmap();
      
      /**
				\brief Read  back the data contained in a buffer and return it in a vector
				\param queue : a pointer to the queue that will be used to copy data to the Buffer
				\param cmdBuff : the command buffer used for this operation, must not be in a recording state
				\param data: the vector on witch the content of the data will be written
      */
      template <typename t>
			void readBack(Queue& queue, CommandBuffer& cmdBuff, std::vector<t>& data){
        Buffer stagingBuffer;

        stagingBuffer.allocate(m_dataSize + m_padding, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        data = std::vector<t>(m_dataSize/sizeof(t));

        cmdBuff.beginRecord();

        setAccess(cmdBuff, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED);

        copyToBuffer(cmdBuff, stagingBuffer, { { 0, 0,  m_dataSize + m_padding } });

        setAccess(cmdBuff, m_stage, m_access, VK_QUEUE_FAMILY_IGNORED);

        cmdBuff.endRecord();

        cmdBuff.submit(queue, {}, {});

        cmdBuff.wait(UINT32_MAX);
        cmdBuff.resetFence();
        void* local_pointer = stagingBuffer.map();

        std::memcpy(&data[0], local_pointer, static_cast<size_t>(m_dataSize));

        stagingBuffer.unmap();
      }

      /**
				\brief Get the Buffer device address,
				\return the address of the buffer on the device
      */
			uint64_t getBufferDeviceAddress();

      /**
				\brief Write a set of data to the buffer,
				The buffer must be host visible for this operation to succeed
      */
			template <typename t>
			void write(const std::vector<t>& data) {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				
				m_mapped = map();

				std::memcpy(m_mapped, &data[0], sizeof(t) * static_cast<size_t>(data.size()));

				std::vector<VkMappedMemoryRange> memory_ranges = {
					{
						VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType    sType
						nullptr,                                // const void       * pNext
						m_bufferMemory,													// VkDeviceMemory     memory
						0,																			// VkDeviceSize       offset
						VK_WHOLE_SIZE                           // VkDeviceSize       size
					}
				};

				VkResult result = vkFlushMappedMemoryRanges(logical, static_cast<uint32_t>(memory_ranges.size()), memory_ranges.data());
				if (VK_SUCCESS != result) {
					std::cout << "Could not flush mapped memory." << std::endl;
				}

				unmap();
			}

			~Buffer() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				if (VK_NULL_HANDLE != m_buffer) {
					vkDestroyBuffer(logical, m_buffer, nullptr);
					m_buffer = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_bufferView) {
					vkDestroyBufferView(logical, m_bufferView, nullptr);
					m_bufferView = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_bufferMemory) {
					vkFreeMemory(logical, m_bufferMemory, nullptr);
					m_bufferMemory = VK_NULL_HANDLE;
				}
			}

      VkPipelineStageFlags getStage(){return m_stage;}
      VkAccessFlagBits     getAccess(){return m_access;}
      
    protected:

      VkBuffer																            m_buffer = VK_NULL_HANDLE;
      VkDeviceMemory													            m_bufferMemory = VK_NULL_HANDLE;
      VkBufferView														            m_bufferView = VK_NULL_HANDLE;


			VkPipelineStageFlags																m_stage;
			VkAccessFlagBits																		m_access;
			uint32_t																						m_queueFamily;
			uint64_t																						m_dataSize = 0;
			uint64_t																						m_padding = 0;

			void*																								m_mapped;


    };


		class TransformBuffer {
		public : 
			TransformBuffer(VkTransformMatrixKHR& transform) {
				m_transformData = transform;

			}

			void allocate(Queue& queue, CommandBuffer& cmdBuff) {
				std::vector<VkTransformMatrixKHR> transform{ m_transformData };
				m_buffer.allocate(queue, cmdBuff, transform, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR));
			}

			Buffer& getBuffer() {
				return m_buffer;
			}

		private :

			VkTransformMatrixKHR m_transformData;

			Buffer m_buffer;



		};
  }
}
