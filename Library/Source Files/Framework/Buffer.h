#pragma once

#include "CommandBuffer.h"
#include "Queue.h"
#include "image.h"

namespace LavaCake {
  namespace Framework {
    
		class Image;

    class Buffer {
    public :

			Buffer();

			Buffer(const Buffer& a);

			template <typename t>
			void allocate(Queue* queue, CommandBuffer& cmdBuff, std::vector<t> rawdata, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_TRANSFER_BIT, VkFormat format = VK_FORMAT_R32_SFLOAT, VkAccessFlagBits accessmod = VK_ACCESS_TRANSFER_WRITE_BIT) {
				Device* d = Device::getDevice();
				VkPhysicalDevice physical = d->getPhysicalDevice();
				VkDevice logical = d->getLogicalDevice();
				m_dataSize = uint64_t(rawdata.size() * sizeof(t));

				m_stage = stageFlagBit;
				m_access = accessmod;
				m_queueFamily = queue->getIndex();

				uint32_t memProp = 0;
				if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
					memProp = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
				}

				if (VK_NULL_HANDLE != *m_buffer) {
					vkDestroyBuffer(logical, *m_buffer, nullptr);
					*m_buffer = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_bufferView) {
					vkDestroyBufferView(logical, *m_bufferView, nullptr);
					*m_bufferView = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_bufferMemory) {
					vkFreeMemory(logical, *m_bufferMemory, nullptr);
					*m_bufferMemory = VK_NULL_HANDLE;
				}


				VkBufferCreateInfo buffer_create_info = {
				VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType
				nullptr,                                // const void           * pNext
				0,                                      // VkBufferCreateFlags    flags
				m_dataSize,                             // VkDeviceSize           size
				VK_BUFFER_USAGE_TRANSFER_DST_BIT| usage,// VkBufferUsageFlags     usage
				VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode
				0,                                      // uint32_t               queueFamilyIndexCount
				nullptr                                 // const uint32_t       * pQueueFamilyIndices
				};

				VkResult result = vkCreateBuffer(logical, &buffer_create_info, nullptr, &*m_buffer);

				if (result != VK_SUCCESS) {
					ErrorCheck::setError("Can't create Buffer");
				}


				VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
				vkGetPhysicalDeviceMemoryProperties(physical, &physical_device_memory_properties);

				VkMemoryRequirements memory_requirements;
				vkGetBufferMemoryRequirements(logical, *m_buffer, &memory_requirements);

				*m_bufferMemory = VK_NULL_HANDLE;
				for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type) {
					if ((memory_requirements.memoryTypeBits & (1 << type)) &&
						((physical_device_memory_properties.memoryTypes[type].propertyFlags & memPropertyFlag) == memPropertyFlag)) {

						VkMemoryAllocateFlagsInfo next{
						VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
						nullptr,
						memProp,
						};

						VkMemoryAllocateInfo buffer_memory_allocate_info = {
							VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
							&next,                                  // const void       * pNext
							memory_requirements.size,                 // VkDeviceSize       allocationSize
							type                          // uint32_t           memoryTypeIndex
						};

						result = vkAllocateMemory(logical, &buffer_memory_allocate_info, nullptr, &*m_bufferMemory);
						if (VK_SUCCESS == result) {
							break;
						}
					}
				}

				if (VK_NULL_HANDLE == *m_bufferMemory) {
					ErrorCheck::setError("Could not allocate memory for a buffer.");
				}

				result = vkBindBufferMemory(logical, *m_buffer, *m_bufferMemory, 0);
				if (VK_SUCCESS != result) {
					ErrorCheck::setError("Could not bind memory object to a buffer.");
				}

				if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
					VkBufferViewCreateInfo buffer_view_create_info = {
					VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,    // VkStructureType            sType
					nullptr,                                      // const void               * pNext
					0,                                            // VkBufferViewCreateFlags    flags
					*m_buffer,                                    // VkBuffer                   buffer
					format,                                       // VkFormat                   format
					0,																						// VkDeviceSize               offset
					VK_WHOLE_SIZE                                 // VkDeviceSize               range
					};

					result = vkCreateBufferView(logical, &buffer_view_create_info, nullptr, &*m_bufferView);
					if (VK_SUCCESS != result) {
						ErrorCheck::setError("Could not creat buffer view.");
					}
				}

				Buffer stagingBuffer;

				stagingBuffer.allocate(m_dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

				stagingBuffer.write(rawdata);

				cmdBuff.resetFence();
				cmdBuff.beginRecord();

				VkPipelineStageFlags																stage = m_stage;
				VkAccessFlagBits																		access = m_access;

				setAccess(cmdBuff, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED);

				stagingBuffer.copyToBuffer(cmdBuff, *this, { { 0, 0, m_dataSize } });

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

			void allocate(uint64_t biteSize, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_TRANSFER_BIT, VkFormat format = VK_FORMAT_R32_SFLOAT);

			void setAccess(CommandBuffer& cmdBuff, VkPipelineStageFlags dstStage, VkAccessFlagBits dstAccessMode, uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED);

			VkBuffer& getHandle();

			VkBufferView& getBufferView();

			void copyToImage(CommandBuffer& cmdBuff, Image& image, std::vector<VkBufferImageCopy> regions);

			void copyToBuffer(CommandBuffer& cmdBuff, Buffer& buffer, std::vector<VkBufferCopy> regions);

			VkDeviceMemory& getMemory();

			void* map();

			void unmap();

			void readBack(Queue* queue, CommandBuffer& cmdBuff, std::vector<float>& data);

			uint64_t getBufferDeviceAddress();

			template <typename t>
			void write(std::vector<t> data) {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				
				m_mapped = map();

				std::memcpy(m_mapped, &data[0], sizeof(t) * static_cast<size_t>(data.size()));

				std::vector<VkMappedMemoryRange> memory_ranges = {
					{
						VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType    sType
						nullptr,                                // const void       * pNext
						*m_bufferMemory,                       // VkDeviceMemory     memory
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
				if (VK_NULL_HANDLE != *m_buffer) {
					vkDestroyBuffer(logical, *m_buffer, nullptr);
					*m_buffer = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_bufferView) {
					vkDestroyBufferView(logical, *m_bufferView, nullptr);
					*m_bufferView = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_bufferMemory) {
					vkFreeMemory(logical, *m_bufferMemory, nullptr);
					*m_bufferMemory = VK_NULL_HANDLE;
				}
			}

    protected:

      VkDestroyer(VkBuffer)																m_buffer;
      VkDestroyer(VkDeviceMemory)													m_bufferMemory;
      VkDestroyer(VkBufferView)														m_bufferView;


			VkPipelineStageFlags																m_stage;
			VkAccessFlagBits																		m_access;
			uint32_t																						m_queueFamily;
			uint64_t																						m_dataSize = 0;

			void*																								m_mapped;


    };


		class TransformBuffer {
		public : 
			TransformBuffer(VkTransformMatrixKHR& transform) {
				m_transformData = transform;

			}

			void allocate(Queue* queue, CommandBuffer& cmdBuff) {
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