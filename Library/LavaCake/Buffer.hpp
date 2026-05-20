#pragma once
#include "./Device.hpp"
#include "./VMAFlags.hpp"
#include <ranges>
#include <utility>

namespace LavaCake {
    /**
     * \brief Manages Vulkan buffers with VMA memory allocation
     */
    class Buffer{
        public :

        Buffer() = default;
        
        /*
        * we delete const copy and const = operator to avoid gpu pointer duplication 
        */
        Buffer(const Buffer& buffer) = delete;
        Buffer& operator=(const Buffer&) = delete;

        /**
         * \brief copie constructeur, copie a buffer and make sure the pointer toward gpu memory are not duplicated
         */
        Buffer(Buffer&& b) noexcept
            : m_buffer(std::exchange(b.m_buffer, VK_NULL_HANDLE)),
            m_allocation(std::exchange(b.m_allocation, {})),
            m_size(std::exchange(b.m_size, 0)),
            m_data(std::exchange(b.m_data, nullptr)),
            m_mapped(std::exchange(b.m_mapped, false)),
            m_device(b.m_device),
            m_allocator(b.m_allocator)
        {}

        /**
         * \brief copie operator, copie a buffer and make sure the pointer toward gpu memory are not duplicated
         */
        Buffer& operator=(Buffer&& b) noexcept
        {
            if (this != &b)
            {
            m_buffer =std::exchange(b.m_buffer, VK_NULL_HANDLE);
            m_allocation =std::exchange(b.m_allocation, {});
            m_size=std::exchange(b.m_size, 0);
            m_data=std::exchange(b.m_data, nullptr);
            m_mapped=std::exchange(b.m_mapped, false);
            m_device = b.m_device;
            m_allocator = b.m_allocator;
            }
            return *this;
        }


        /**
         * \brief Create and allocate a buffer
         * \param device the device on which the buffer will be created
         * \param size the size of the buffer in byte
         * \param usage the buffer usage
         * \param memoryFlags the memory requirements
         */
        Buffer(const LavaCake::Device& device, VkDeviceSize size, vk::BufferUsageFlags usage, vk::AllocationCreateFlags memoryFlags= vk::AllocationCreateFlagBits::eCreateDedicatedMemory) : 
            Buffer(device, device.getAllocator(), size,  usage,  memoryFlags){}


        /**
         * \brief Create and allocate a buffer
         * \param device the device on which the buffer will be created
         * \param allocator the vma allocator which will be used to allocate the buffer 
         * \param size the size of the buffer in byte
         * \param usage the buffer usage
         * \param memoryFlags the memory requirements
         */
        Buffer(const vk::Device& device, const VmaAllocator& allocator, VkDeviceSize size, vk::BufferUsageFlags usage, vk::AllocationCreateFlags memoryFlags= vk::AllocationCreateFlagBits::eCreateDedicatedMemory){
           init(device, allocator, size,usage,memoryFlags);
        }

        /**
         * \brief Create and allocate a buffer wiht predefined data using a throw away staging buffer
         * \param device the device on which the buffer will be created
         * \param data the data to initialize the buffer with
         * \param usage the buffer usage
         * \param memoryFlags the memory requirements
         */

        template <std::ranges::contiguous_range Range>
        Buffer(const LavaCake::Device& device, const Range& data, vk::BufferUsageFlags usage, vk::AllocationCreateFlags memoryFlags = vk::AllocationCreateFlagBits::eCreateDedicatedMemory) : 
            Buffer(device, device.getAllocator(), device.getAnyQueue(), device.getCommandPool() , data,  usage,  memoryFlags){}


        /**
         * \brief Create and allocate a buffer wiht predefined data using a throw away staging buffer
         * \param device the device on which the buffer will be created
         * \param allocator the vma allocator which will be used to allocate the buffer 
         * \param queue the queue that will be used to copy the data to the buffer 
         * \param commandPool the commandPool that will be used to create the commandbuffer used for the copy operation 
         * \param data the data to initialize the buffer with
         * \param usage the buffer usage
         * \param memoryFlags the memory requirements
         */
        template <std::ranges::contiguous_range Range>
        Buffer(const vk::Device& device, const VmaAllocator& allocator, const vk::Queue& queue, const vk::CommandPool& commandPool, const Range& data, vk::BufferUsageFlags usage, vk::AllocationCreateFlags memoryFlags = vk::AllocationCreateFlagBits::eCreateDedicatedMemory) {
            using T = std::ranges::range_value_t<Range>;
            vk::DeviceSize bufferSize = std::ranges::size(data) * sizeof(T);
            if(memoryFlags & vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite){
                init(device,allocator,bufferSize,usage,memoryFlags);
                map();
                memcpy(m_data, std::ranges::data(data), m_size);
                unmap();
            }else{

                init(device,allocator,bufferSize,usage | vk::BufferUsageFlagBits::eTransferDst,memoryFlags);
                Buffer staging(device,allocator, bufferSize, usage | vk::BufferUsageFlagBits::eTransferSrc, memoryFlags | vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

                void* staggingMemory = staging.map();
                // Copy data
                memcpy(staggingMemory, std::ranges::data(data), bufferSize);
                staging.unmap();


                auto cmd = LavaCake::CommandBuffer(device,commandPool,false);

                
                cmd.begin();

                copyFromBuffer(cmd,staging);

                cmd.end();

                vk::SubmitInfo submitInfo{};
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = cmd;
                queue.submit(submitInfo);
                queue.waitIdle();

            }
        }

        /**
         * \brief Copy the buffer's data to another buffer
         * \param cmd the command buffer to register the copy operation into
         * \param dstBuffer the buffer to copy the data to
         * \param srcOffset byte offset into this buffer
         * \param dstOffset byte offset into dstBuffer
         * \param size number of bytes to copy (0 = copy all)
         */
        void copyToBuffer(vk::CommandBuffer cmd, Buffer& dstBuffer, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0, VkDeviceSize size = 0) {
            vk::BufferCopy copyRegion{};
            copyRegion.srcOffset = srcOffset;
            copyRegion.dstOffset = dstOffset;
            copyRegion.size = size == 0 ? m_size : size;
            cmd.copyBuffer(m_buffer, dstBuffer.m_buffer, copyRegion);
        }

        /**
         * \brief Copy the data from another buffer
         * \param cmd the command buffer to register the copy operation into
         * \param srcBuffer the buffer to copy the data from
         * \param srcOffset byte offset into srcBuffer
         * \param dstOffset byte offset into this buffer
         * \param size number of bytes to copy (0 = copy all)
         */
        void copyFromBuffer(vk::CommandBuffer cmd, Buffer& srcBuffer, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0, VkDeviceSize size = 0) {
            vk::BufferCopy copyRegion{};
            copyRegion.srcOffset = srcOffset;
            copyRegion.dstOffset = dstOffset;
            copyRegion.size = size == 0 ? m_size : size;
            cmd.copyBuffer(srcBuffer.m_buffer, m_buffer, copyRegion);
        }

        /**
         * \brief map the memory of the buffer to a host visible pointer
         * \return the pointer toward the mapped memory
         */
        void* map(){
            if(m_mapped){
                return m_data;
            }else{
                vmaMapMemory(m_allocator, m_allocation, &m_data); 
                m_mapped = true;
                return m_data;
            }
        }

        /**
         * \brief unmap the memory of the buffer
         */
        void unmap(){
            if(m_mapped){
                vmaUnmapMemory(m_allocator, m_allocation); 
                m_mapped = false;
            }
        }

        /**
         * \brief Destructor - unmaps and destroys the buffer
         */
        ~Buffer(){
            if(m_mapped){
                unmap();
            }
            vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
        }

        /**
         * \brief Get the Vulkan buffer handle
         * \return the vk::Buffer handle
         */
        vk::Buffer getBuffer() const {
            return m_buffer;
        }

        /**
         * \brief Get the buffer size in bytes
         */
        VkDeviceSize size() const {
            return m_size;
        }

        /**
         * \brief Get the device address of the buffer
         * \param device the logical device (must have bufferDeviceAddress feature enabled)
         */
        vk::DeviceAddress getDeviceAddress(const vk::Device& device) const {
            vk::BufferDeviceAddressInfo info{};
            info.buffer = m_buffer;
            return device.getBufferAddress(info);
        }

        /**
         * \brief Implicit conversion to vk::Buffer
         */
        operator vk::Buffer() const {
            return m_buffer;
        }

        /**
         * \brief Implicit conversion to vk::Buffer pointer
         */
        operator const vk::Buffer*() const {
            return &m_buffer;
        }

        private :
        VkDeviceSize                    m_size;         ///< Buffer size in bytes
        vk::Buffer                      m_buffer;       ///< The Vulkan buffer handle
        VmaAllocation                   m_allocation;   ///< VMA allocation handle

        void*                           m_data;         ///< Mapped memory pointer
        bool                            m_mapped = false; ///< Whether the buffer is currently mapped

        vk::Device                      m_device;        ///< Associated device
        VmaAllocator                    m_allocator;     ///< Associated allocator


        /**
         * \brief Create and allocate a buffer 
         * This is only to be called by the constructor of the class
         * \param device the device on which the buffer will be created
         * \param allocator the vma allocator which will be used to allocate the buffer 
         * \param size the size of the buffer in byte
         * \param usage the buffer usage
         * \param memoryFlags the memory requirements
         */

        void init(const vk::Device& device, VmaAllocator allocator,  VkDeviceSize size, vk::BufferUsageFlags usage,  vk::AllocationCreateFlags memoryFlags){
            m_size = size;
            vk::BufferCreateInfo bufferInfo{};
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = vk::SharingMode::eExclusive;
            m_allocator = allocator;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = (VmaAllocationCreateFlags)memoryFlags;

            VkBuffer buffer;

            if (vmaCreateBuffer(
                    m_allocator,
                    reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
                    &allocInfo,
                    &buffer,
                    &m_allocation,
                    nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create VMA buffer");
            }

            m_buffer = vk::Buffer(buffer);

            //std::cout << "VMA buffer allocated.\n";

            m_device = device;
        }
    };
    

}