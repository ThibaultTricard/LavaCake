#pragma once


namespace LavaCake{
    
    class SwapChainImage{
    private:
        vk::Image m_image;
        vk::ImageView m_imageView;
        uint32_t  m_index;
    public:
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

        void prepareForAttachement(vk::CommandBuffer& cmdBuffer){
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

        void prepareForPresent(vk::CommandBuffer& cmdBuffer){
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


        vk::ImageView& getView(){return m_imageView;}

        vk::Image& getImage(){return m_image;}

        uint32_t getIndex(){return m_index;}


        ~SwapChainImage(){}
    };

} // namespace LavaCake
