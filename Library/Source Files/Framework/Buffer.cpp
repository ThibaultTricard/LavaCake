#include "Buffer.h"

namespace LavaCake{
	namespace Framework {


		Buffer::Buffer() {

		}

		Buffer::Buffer(const Buffer& a) {
			m_buffer = (a.m_buffer);
			m_bufferMemory = (a.m_bufferMemory);
			m_bufferView = (a.m_bufferView);

			m_dataSize = a.m_dataSize;
			m_padding = a.m_padding;
		}

		void Buffer::allocate(uint64_t byteSize, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memPropertyFlag , VkPipelineStageFlagBits stageFlagBit , VkFormat format ) {
			Device* d = Device::getDevice();
			VkPhysicalDevice physical = d->getPhysicalDevice();
			VkDevice logical = d->getLogicalDevice();
			m_dataSize = byteSize;

			VkPhysicalDeviceProperties p;
			vkGetPhysicalDeviceProperties(physical,&p);

			m_padding = p.limits.nonCoherentAtomSize - m_dataSize % p.limits.nonCoherentAtomSize;

			m_stage = stageFlagBit;
			m_access = VkAccessFlagBits(0);

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
			usage|VK_BUFFER_USAGE_TRANSFER_DST_BIT, // VkBufferUsageFlags     usage
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
				m_buffer,                                    // VkBuffer                   buffer
				format,                                       // VkFormat                   format
				0,																						// VkDeviceSize               offset
				VK_WHOLE_SIZE                                 // VkDeviceSize               range
				};

				result = vkCreateBufferView(logical, &buffer_view_create_info, nullptr, &m_bufferView);
				if (VK_SUCCESS != result) {
					ErrorCheck::setError("Could not creat buffer view.");
				}
			}

		}

		void Buffer::setAccess(CommandBuffer& cmdBuff, VkPipelineStageFlags dstStage, VkAccessFlagBits dstAccessMode, uint32_t dstQueueFamily ) {

			VkBufferMemoryBarrier bufferMemoryBarrier{};
			bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferMemoryBarrier.buffer = m_buffer;
			bufferMemoryBarrier.srcAccessMask = m_access;
			bufferMemoryBarrier.dstAccessMask = dstAccessMode;

			if (dstQueueFamily == VK_QUEUE_FAMILY_IGNORED) {
				bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			}
			else {
				bufferMemoryBarrier.srcQueueFamilyIndex = m_queueFamily;
			}

			bufferMemoryBarrier.dstQueueFamilyIndex = dstQueueFamily;
			bufferMemoryBarrier.offset = 0;
			bufferMemoryBarrier.size = VK_WHOLE_SIZE;


			vkCmdPipelineBarrier(cmdBuff.getHandle(), m_stage, dstStage, 0, 0, nullptr, 1,  &bufferMemoryBarrier , 0, nullptr);


			m_stage = dstStage;
			if (dstQueueFamily != VK_QUEUE_FAMILY_IGNORED) {
				m_queueFamily = dstQueueFamily;
			}
			m_access = dstAccessMode;

		}

		VkBuffer& Buffer::getHandle() {
			return m_buffer;
		}

		VkBufferView& Buffer::getBufferView() {
			return m_bufferView;
		}

    void Buffer::copyToImage(CommandBuffer& cmdBuff, Image& image, const VkBufferImageCopy& region) {
      vkCmdCopyBufferToImage(cmdBuff.getHandle(), m_buffer, image.getHandle(), image.getLayout(), static_cast<uint32_t>(1), &region);
    }

    void Buffer::copyToImage(CommandBuffer& cmdBuff, Image& image, const std::span<VkBufferImageCopy>& regions) {
			if (regions.size() > 0) {
				vkCmdCopyBufferToImage(cmdBuff.getHandle(), m_buffer, image.getHandle(), image.getLayout(), static_cast<uint32_t>(regions.size()), regions.data());
			}
		}

    void Buffer::copyToBuffer(CommandBuffer& cmdBuff, Buffer& buffer, const VkBufferCopy& region) {
      vkCmdCopyBuffer(cmdBuff.getHandle(), m_buffer, buffer.getHandle(), static_cast<uint32_t>(1), &region);
    }

    void Buffer::copyToBuffer(CommandBuffer& cmdBuff, Buffer& buffer, const std::span<VkBufferCopy>& regions) {
			if (regions.size() > 0) {
				vkCmdCopyBuffer(cmdBuff.getHandle(), m_buffer, buffer.getHandle(), static_cast<uint32_t>(regions.size()), regions.data());
			}
		}

		VkDeviceMemory& Buffer::getMemory() {
			return m_bufferMemory;
		}

		void* Buffer::map() {
			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			VkResult result = vkMapMemory(logical, m_bufferMemory, 0, m_dataSize + m_padding, 0, &m_mapped);

			if (VK_SUCCESS != result) {
				//std::cout << "Could not map memory object." << std::endl;
				//return false;
			}

			return m_mapped;
		}

		void Buffer::unmap() {
			Device* d = Device::getDevice();
			VkDevice logical = d->getLogicalDevice();
			vkUnmapMemory(logical, m_bufferMemory);
		}

		uint64_t Buffer::getBufferDeviceAddress()
		{
			Framework::Device* d = Framework::Device::getDevice();
			VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
			bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufferDeviceAI.buffer = m_buffer;
			return vkGetBufferDeviceAddressKHR(d->getLogicalDevice(), &bufferDeviceAI);
		}
	}
}
