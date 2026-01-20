#pragma once
#include "./Device.hpp"
#include "./VMAFlags.hpp"
#include "./Buffer.hpp"
#include <span>

namespace LavaCake {
    class ImageView; 
    class Image {
    public:

        Image() = default;
        
        /*
        * Delete copy constructor and copy assignment to avoid GPU pointer duplication
        */
        Image(const Image& image) = delete;
        Image& operator=(const Image&) = delete;

        /**
         * \brief Move constructor - transfers ownership without duplicating GPU resources
         */
        Image(Image&& img) noexcept
            : m_image(std::exchange(img.m_image, VK_NULL_HANDLE)),
              //m_imageView(std::exchange(img.m_imageView, VK_NULL_HANDLE)),
              m_allocation(std::exchange(img.m_allocation, {})),
              m_width(std::exchange(img.m_width, 0)),
              m_height(std::exchange(img.m_height, 0)),
              m_depth(std::exchange(img.m_depth, 1)),
              m_mipLevels(std::exchange(img.m_mipLevels, 1)),
              m_arrayLayers(std::exchange(img.m_arrayLayers, 1)),
              m_format(std::exchange(img.m_format, vk::Format::eUndefined)),
              m_device(img.m_device)
        {}

        /**
         * \brief Move assignment - transfers ownership without duplicating GPU resources
         */
        Image& operator=(Image&& img) noexcept {
            if (this != &img) {
                // Clean up existing resources
                cleanup();
                
                m_image = std::exchange(img.m_image, VK_NULL_HANDLE);
                //m_imageView = std::exchange(img.m_imageView, VK_NULL_HANDLE);
                m_allocation = std::exchange(img.m_allocation, {});
                m_width = std::exchange(img.m_width, 0);
                m_height = std::exchange(img.m_height, 0);
                m_depth = std::exchange(img.m_depth, 1);
                m_mipLevels = std::exchange(img.m_mipLevels, 1);
                m_arrayLayers = std::exchange(img.m_arrayLayers, 1);
                m_format = std::exchange(img.m_format, vk::Format::eUndefined);
                m_device = img.m_device;
            }
            return *this;
        }

        /**
         * \brief Create and allocate an image
         * \param device The device on which the image will be created
         * \param width Image width in pixels
         * \param height Image height in pixels
         * \param format Image format
         * \param usage Image usage flags
         * \param memoryFlags Memory allocation flags
         * \param mipLevels Number of mipmap levels (default: 1)
         * \param arrayLayers Number of array layers (default: 1)
         * \param depth Image depth for 3D images (default: 1)
         * \param imageType Image type (default: 2D)
         */
        Image(const LavaCake::Device& device,
              uint32_t width,
              uint32_t height,
              uint32_t depth,
              vk::Format format,
              vk::ImageUsageFlags usage,
              vk::AllocationCreateFlags memoryFlags = vk::AllocationCreateFlagBits::eCreateDedicatedMemory,
              uint32_t mipLevels = 1,
              uint32_t arrayLayers = 1)
        {
            init(device, width, height, depth, format, usage, memoryFlags, mipLevels, arrayLayers);
        }

        /**
         * \brief Create and initialize an image with data from a staging buffer
         * \param device The device on which the image will be created
         * \param width Image width in pixels
         * \param height Image height in pixels
         * \param format Image format
         * \param data The pixel data to upload
         * \param usage Image usage flags
         * \param memoryFlags Memory allocation flags
         */
        template <typename T>
        Image(const LavaCake::Device& device,
              const std::vector<T>& data,
              uint32_t width,
              uint32_t height,
              uint32_t depth,
              vk::Format format,
              vk::ImageUsageFlags usage,
              vk::AllocationCreateFlags memoryFlags = vk::AllocationCreateFlagBits::eCreateDedicatedMemory,
              uint32_t mipLevels = 1,
              uint32_t arrayLayers = 1)
        {
            init(device, width, height, depth, format, usage | vk::ImageUsageFlagBits::eTransferDst, memoryFlags, mipLevels, arrayLayers);

            // Create staging buffer
            vk::DeviceSize imageSize = width * height * depth * sizeof(T);
            Buffer staging(device, data, vk::BufferUsageFlagBits::eTransferSrc,
                         vk::AllocationCreateFlagBits::eCreateHostAccessSequentialWrite);

            // Transition to transfer dst layout and copy
            vk::CommandBuffer cmd = device.allocateCommandBuffer();
            vk::CommandBufferBeginInfo beginInfo{};
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
            cmd.begin(beginInfo);

            transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
            copyFromBuffer(cmd, staging);
            transitionLayout(cmd, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

            cmd.end();

            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;
            auto queue = device.getAnyQueue();
            queue.submit(submitInfo);
            queue.waitIdle();

            device.freeCommandBuffer(cmd);
        }

        /**
         * \brief Copy data from a buffer to this image
         * \param cmd Command buffer to record the copy operation
         * \param srcBuffer Source buffer containing image data
         */
        void copyFromBuffer(vk::CommandBuffer& cmd, Buffer& srcBuffer) {
            vk::BufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = m_arrayLayers;
            region.imageOffset = vk::Offset3D{0, 0, 0};
            region.imageExtent = vk::Extent3D{m_width, m_height, m_depth};

            cmd.copyBufferToImage(srcBuffer.getBuffer(), m_image, vk::ImageLayout::eTransferDstOptimal, region);
        }

        /**
         * \brief Copy this image's data to a buffer
         * \param cmd Command buffer to record the copy operation
         * \param dstBuffer Destination buffer
         */
        void copyToBuffer(vk::CommandBuffer& cmd, Buffer& dstBuffer) {
            vk::BufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = m_arrayLayers;
            region.imageOffset = vk::Offset3D{0, 0, 0};
            region.imageExtent = vk::Extent3D{m_width, m_height, m_depth};

            cmd.copyImageToBuffer(m_image, vk::ImageLayout::eTransferSrcOptimal, dstBuffer.getBuffer(), region);
        }

        /**
         * \brief Transition image layout
         * \param cmd Command buffer to record the transition
         * \param oldLayout Current layout
         * \param newLayout Target layout
         */
        void transitionLayout(vk::CommandBuffer& cmd, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_image;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = m_mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = m_arrayLayers;

            vk::PipelineStageFlags srcStage;
            vk::PipelineStageFlags dstStage;

            if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
                dstStage = vk::PipelineStageFlagBits::eTransfer;
            } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                srcStage = vk::PipelineStageFlagBits::eTransfer;
                dstStage = vk::PipelineStageFlagBits::eFragmentShader;
            } else {
                throw std::invalid_argument("Unsupported layout transition");
            }

            cmd.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags{}, nullptr, nullptr, barrier);
        }

        /**
         * \brief Create an image view for this image
         * \param viewType Type of image view (default: 2D)
         * \param aspectMask Aspect mask (default: Color)
         */
        /*void createImageView(vk::ImageViewType viewType = vk::ImageViewType::e2D,
                           vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor) {
            if (m_imageView) {
                m_device.getDevice().destroyImageView(m_imageView);
            }

            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = m_image;
            viewInfo.viewType = viewType;
            viewInfo.format = m_format;
            viewInfo.subresourceRange.aspectMask = aspectMask;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = m_mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = m_arrayLayers;

            m_imageView = m_device.getDevice().createImageView(viewInfo);
        }*/

        ~Image() {
            cleanup();
        }

        vk::Image getImage() const { return m_image; }
        
        
        operator vk::Image() const { return m_image; }
        

        uint32_t getWidth() const { return m_width; }
        uint32_t getHeight() const { return m_height; }
        uint32_t getDepth() const { return m_depth; }
        vk::Format getFormat() const { return m_format; }

        friend class ImageView;

    private:
        vk::Image m_image;
        VmaAllocation m_allocation;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_depth = 1;
        uint32_t m_mipLevels = 1;
        uint32_t m_arrayLayers = 1;
        vk::Format m_format = vk::Format::eUndefined;

        LavaCake::Device m_device = LavaCake::Device();

        /**
         * \brief Initialize and allocate an image
         */
        void init(const LavaCake::Device& device,
                 uint32_t width,
                 uint32_t height,
                 uint32_t depth,
                 vk::Format format,
                 vk::ImageUsageFlags usage,
                 vk::AllocationCreateFlags memoryFlags,
                 uint32_t mipLevels,
                 uint32_t arrayLayers
                )
        {
            m_width = width;
            m_height = height;
            m_depth = depth;
            m_format = format;
            m_mipLevels = mipLevels;
            m_arrayLayers = arrayLayers;

            vk::ImageType imageType = vk::ImageType::e1D;
            if(m_height > 1){
                imageType = vk::ImageType::e2D;
                if(m_depth > 1){
                    imageType = vk::ImageType::e3D;
                }
            }

            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = imageType;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = depth;
            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = arrayLayers;
            imageInfo.format = format;
            imageInfo.tiling = vk::ImageTiling::eOptimal;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;
            imageInfo.usage = usage;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            imageInfo.samples = vk::SampleCountFlagBits::e1;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = (VmaAllocationCreateFlags)memoryFlags;

            VkImage image;
            if (vmaCreateImage(
                    device.getAllocator(),
                    reinterpret_cast<VkImageCreateInfo*>(&imageInfo),
                    &allocInfo,
                    &image,
                    &m_allocation,
                    nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create VMA image");
            }

            m_image = vk::Image(image);
            std::cout << "VMA image allocated.\n";

            m_device = device;
        }

        /**
         * \brief Clean up image and image view resources
         */
        void cleanup() {
            if (m_image) {
                vmaDestroyImage(m_device.getAllocator(), m_image, m_allocation);
                m_image = VK_NULL_HANDLE;
            }
        }
    };


    class ImageView {

    private:
        vk::ImageView m_imageView;

        LavaCake::Device m_device = LavaCake::Device();

        ImageView() = default;


        ImageView (const Image& image,
                           vk::ImageViewType viewType = vk::ImageViewType::e2D,
                           vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor){
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = image.m_image;
            viewInfo.viewType = viewType;
            viewInfo.format = image.m_format;
            viewInfo.subresourceRange.aspectMask = aspectMask;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = image.m_mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = image.m_arrayLayers;

            m_imageView = image.m_device.getDevice().createImageView(viewInfo);
            m_device = image.m_device;
        }

        ImageView (const Image& image,
                           vk::ImageViewType viewType,
                           vk::ImageAspectFlags aspectMask,
                           vk::Format format,
                           uint32_t baseMipLevel, uint32_t levelCount, 
                           uint32_t baseArrayLayer, uint32_t layerCount){
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = image.m_image;
            viewInfo.viewType = viewType;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectMask;
            viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
            viewInfo.subresourceRange.levelCount = levelCount;
            viewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
            viewInfo.subresourceRange.layerCount = layerCount;

            m_imageView = image.m_device.getDevice().createImageView(viewInfo);
            m_device = image.m_device;
        }
      
        
        operator vk::ImageView() const { return m_imageView; }
        vk::ImageView getImageView() const { return m_imageView; }

        ~ImageView(){
            if (m_imageView) {
                m_device.getDevice().destroyImageView(m_imageView);
                m_imageView = VK_NULL_HANDLE;
            }
        }
    };

    class Sampler{
    public:
        Sampler() = default;

        // Delete copy to prevent duplicate handles
        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        /**
         * \brief Move constructor
         */
        Sampler(Sampler&& s) noexcept
            : m_sampler(std::exchange(s.m_sampler, VK_NULL_HANDLE)),
              m_device(std::move(s.m_device))
        {}

        /**
         * \brief Move assignment
         */
        Sampler& operator=(Sampler&& s) noexcept {
            if (this != &s) {
                cleanup();
                m_sampler = std::exchange(s.m_sampler, VK_NULL_HANDLE);
                m_device = std::move(s.m_device);
            }
            return *this;
        }

        /**
         * \brief Create a sampler with specified parameters
         * \param device The device on which to create the sampler
         * \param mipmapMode Mipmap filtering mode
         * \param minLod Minimum LOD level
         * \param maxLod Maximum LOD level
         * \param mipLodBias LOD bias
         * \param anisotropyEnable Enable anisotropic filtering
         * \param maxAnisotropy Maximum anisotropy level
         * \param magFilter Magnification filter
         * \param minFilter Minification filter
         * \param addressModeU U coordinate addressing mode
         * \param addressModeV V coordinate addressing mode
         * \param addressModeW W coordinate addressing mode
         */
        Sampler(const LavaCake::Device& device,
                vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear,
                float minLod = 0.0f,
                float maxLod = VK_LOD_CLAMP_NONE,
                float mipLodBias = 0.0f,
                bool anisotropyEnable = true,
                float maxAnisotropy = 16.0f,
                vk::Filter magFilter = vk::Filter::eLinear,
                vk::Filter minFilter = vk::Filter::eLinear,
                vk::SamplerAddressMode addressModeU = vk::SamplerAddressMode::eRepeat,
                vk::SamplerAddressMode addressModeV = vk::SamplerAddressMode::eRepeat,
                vk::SamplerAddressMode addressModeW = vk::SamplerAddressMode::eRepeat
                )
        {
            init(device, magFilter, minFilter, addressModeU, addressModeV, addressModeW,
                 anisotropyEnable, maxAnisotropy, mipmapMode, minLod, maxLod, mipLodBias);
        }

        /**
         * \brief Create a sampler from a full SamplerCreateInfo
         */
        Sampler(const LavaCake::Device& device, const vk::SamplerCreateInfo& createInfo) {
            m_device = device;
            m_sampler = m_device.getDevice().createSampler(createInfo);
            std::cout << "Sampler created.\n";
        }

        ~Sampler() {
            cleanup();
        }

        vk::Sampler getSampler() const { return m_sampler; }
        operator vk::Sampler() const { return m_sampler; }

        bool isValid() const { return m_sampler != VK_NULL_HANDLE; }

    private:
        vk::Sampler m_sampler;
        LavaCake::Device m_device;

        void init(const LavaCake::Device& device,
                 vk::Filter magFilter,
                 vk::Filter minFilter,
                 vk::SamplerAddressMode addressModeU,
                 vk::SamplerAddressMode addressModeV,
                 vk::SamplerAddressMode addressModeW,
                 bool anisotropyEnable,
                 float maxAnisotropy,
                 vk::SamplerMipmapMode mipmapMode,
                 float minLod,
                 float maxLod,
                 float mipLodBias)
        {
            m_device = device;

            // Clamp anisotropy to device limits
            vk::PhysicalDeviceProperties properties = m_device.getPhysicalDevice().getProperties();
            if (anisotropyEnable && maxAnisotropy > properties.limits.maxSamplerAnisotropy) {
                maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            }

            vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter = magFilter;
            samplerInfo.minFilter = minFilter;
            samplerInfo.addressModeU = addressModeU;
            samplerInfo.addressModeV = addressModeV;
            samplerInfo.addressModeW = addressModeW;
            samplerInfo.anisotropyEnable = anisotropyEnable;
            samplerInfo.maxAnisotropy = maxAnisotropy;
            samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = vk::CompareOp::eAlways;
            samplerInfo.mipmapMode = mipmapMode;
            samplerInfo.mipLodBias = mipLodBias;
            samplerInfo.minLod = minLod;
            samplerInfo.maxLod = maxLod;

            m_sampler = m_device.getDevice().createSampler(samplerInfo);
            std::cout << "Sampler created.\n";
        }

        void cleanup() {
            if (m_sampler) {
                m_device.getDevice().destroySampler(m_sampler);
                m_sampler = VK_NULL_HANDLE;
            }
        }
    };

} // namespace LavaCake