#pragma once
#include "./Device.hpp"
#include <span>

namespace LavaCake {

    class Buffer{
        public :

        Buffer() = default;
        
        /*
        * we delete const copy and const = operator to avoid gpu pointer duplication 
        */
        Buffer(const Buffer& buffer) = delete;
        Buffer& operator=(const Buffer&) = delete;

        /**
         * \brief copie constructeur, copie a buffer and make sure the poiter toward gpu memory are not duplicated
         */
        Buffer(Buffer&& b) noexcept
            : m_buffer(std::exchange(b.m_buffer, VK_NULL_HANDLE)),
            m_allocation(std::exchange(b.m_allocation, {})),
            m_size(std::exchange(b.m_size, 0)),
            m_data(std::exchange(b.m_data, nullptr)),
            m_mapped(std::exchange(b.m_mapped, false))
        {}

        /**
         * \brief copie operator, copie a buffer and make sure the poiter toward gpu memory are not duplicated
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
        Buffer(const LavaCake::Device& device, VkDeviceSize size, vk::BufferUsageFlags usage,  VmaAllocationCreateFlags memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT){
           init(device,size,usage,memoryFlags);
        }


        /**
         * \brief Create and allocate a buffer wiht predefined data using a throw away staging buffer
         * \param device the device on which the buffer will be created
         * \param data the data to initialize the buffer with
         * \param usage the buffer usage
         * \param memoryFlags the memory requirements
         */
        template <typename T>
        Buffer(const LavaCake::Device& device, const std::vector<T>& data, vk::BufferUsageFlags usage, VmaAllocationCreateFlags memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) {
            vk::DeviceSize bufferSize = data.size()*sizeof(T);
            if(memoryFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT){
                init(device,bufferSize,usage,memoryFlags);
                map();
                memcpy(m_data, &data[0], m_size);
                unmap();
            }else{

                init(device,bufferSize,usage | vk::BufferUsageFlagBits::eTransferDst,memoryFlags);
                Buffer staging(device, data.size()*sizeof(T) , usage | vk::BufferUsageFlagBits::eTransferSrc, memoryFlags | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
               
                void* staggingMemory = staging.map();
                // Copy data
                memcpy(staggingMemory, data.data(), bufferSize);
                staging.unmap();

                
                vk::CommandBuffer cmd = device.allocateCommandBuffer();

                vk::CommandBufferBeginInfo beginInfo{};
                beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
                cmd.begin(beginInfo);

                copyFromBuffer(cmd,staging);

                cmd.end();

                vk::SubmitInfo submitInfo{};
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &cmd;
                auto queue = device.getAnyQueue();
                queue.submit(submitInfo);
                queue.waitIdle();
            }
        }

        /**
         * \brief Copy the buffer's data to another buffer
         * \param cmd the command buffer to register the copy operation into
         * \param dstBuffer the buffer to copy the data to
         */
        void copyToBuffer(vk::CommandBuffer&cmd, Buffer& dstBuffer){
            vk::BufferCopy copyRegion{};
            copyRegion.size = m_size;

            cmd.copyBuffer(
                m_buffer,
                dstBuffer. m_buffer,
                copyRegion
            );
        }

        /**
         * \brief Copy the data from another buffer 
         * \param cmd the command buffer to register the copy operation into
         * \param srcBuffer the buffer to copy the data from
         */
        void copyFromBuffer(vk::CommandBuffer&cmd, Buffer& srcBuffer){
            vk::BufferCopy copyRegion{};
            copyRegion.size = m_size;

            cmd.copyBuffer(
                srcBuffer.m_buffer,
                m_buffer,
                copyRegion
            );
        }

        /**
         * \brief map the memory of the buffer to a host visible pointer
         * \return the pointer toward the mapped memory
         */
        void* map(){
            if(m_mapped){
                return m_data;
            }else{
                vmaMapMemory(m_device->getAllocator(), m_allocation, &m_data); 
                m_mapped = true;
                return m_data;
            }
        }

        /**
         * \brief unmap the memory of the buffer
         */
        void unmap(){
            if(m_mapped){
                vmaUnmapMemory(m_device->getAllocator(), m_allocation); 
                m_mapped = false;
            }
        }


        ~Buffer(){
            if(m_mapped){
                unmap();
            }
            vmaDestroyBuffer(m_device->getAllocator(), m_buffer, m_allocation);
        }

        private :
        VkDeviceSize                    m_size;
        vk::Buffer                      m_buffer;
        VmaAllocation                   m_allocation;

        void*                           m_data;
        bool                            m_mapped = false;

        const LavaCake::Device* m_device;


        /**
         * \brief Create and allocate a buffer 
         * This is only to be called by the constructor of the class
         * \param device the device on which the buffer will be created
         * \param size the size of the buffer in byte
         * \param usage the buffer usage
         * \param memoryFlags the memory requirements
         */

        void init(const LavaCake::Device& device, VkDeviceSize size, vk::BufferUsageFlags usage,  VmaAllocationCreateFlags memoryFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT){
            m_size = size;
            vk::BufferCreateInfo bufferInfo{};
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = vk::SharingMode::eExclusive;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = memoryFlags;

            VkBuffer buffer;

            if (vmaCreateBuffer(
                    device.getAllocator(),
                    reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
                    &allocInfo,
                    &buffer,
                    &m_allocation,
                    nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create VMA buffer");
            }

            m_buffer = vk::Buffer(buffer);

            std::cout << "VMA buffer allocated.\n";

            m_device = &device;
        }
    };
    

}