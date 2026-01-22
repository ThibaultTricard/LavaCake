#pragma once


namespace LavaCake{

    /**
     * \brief Represents a swapchain image with its associated view and index
     */
    class SwapChainImage{
    private:
        vk::Image m_image;          ///< The Vulkan image handle
        vk::ImageView m_imageView;  ///< The image view for rendering
        uint32_t  m_index;          ///< The index of this image in the swapchain
    public:
        /**
         * \brief Constructs a SwapChainImage and creates its image view
         * \param device the Vulkan device
         * \param image the Vulkan image handle
         * \param index the index of this image in the swapchain
         * \param format the image format
         */
        SwapChainImage(vk::Device device, vk::Image image,uint32_t index, vk::Format format){
            m_image = image;
            m_index = index;
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = image;
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = format;
            viewInfo.subresourceRange = {
                vk::ImageAspectFlagBits::eColor,
                0, 1,
                0, 1
            };

            m_imageView = device.createImageView(viewInfo);
        }

        /**
         * \brief Transition the image layout to prepare for use as a color attachment
         * \param cmdBuffer the command buffer to record the barrier into
         */
        void prepareForAttachementBarrier(vk::CommandBuffer& cmdBuffer){
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eUndefined;
            barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_image;
            barrier.subresourceRange = {
                vk::ImageAspectFlagBits::eColor,
                0, 1,
                0, 1
            };

            barrier.srcAccessMask = {}; // nothing to wait on
            barrier.dstAccessMask =
                vk::AccessFlagBits::eColorAttachmentWrite;

            cmdBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {},
                nullptr,
                nullptr,
                barrier
            );
        }

        /**
         * \brief Transition the image layout to prepare for presentation to the screen
         * \param cmdBuffer the command buffer to record the barrier into
         */
        void prepareForPresentBarrier(vk::CommandBuffer& cmdBuffer){
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
            barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_image;
            barrier.subresourceRange = {
                vk::ImageAspectFlagBits::eColor,
                0, 1,
                0, 1
            };

            barrier.srcAccessMask =
                vk::AccessFlagBits::eColorAttachmentWrite;
            barrier.dstAccessMask = {};

            cmdBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {},
                nullptr,
                nullptr,
                barrier
            );
        }

        /**
         * \brief Returns the image view for rendering
         * \return reference to the vk::ImageView
         */
        vk::ImageView& getView(){return m_imageView;}

        /**
         * \brief Returns the underlying Vulkan image
         * \return reference to the vk::Image
         */
        vk::Image& getImage(){return m_image;}

        /**
         * \brief Returns the swapchain index of this image
         * \return the index value
         */
        uint32_t getIndex(){return m_index;}

        /**
         * \brief Destructor
         */
        ~SwapChainImage(){}
    };

} // namespace LavaCake
