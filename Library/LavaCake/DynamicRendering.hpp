#pragma once

#include "Device.hpp"
#include <vector>
#include <optional>
#include <stdexcept>

namespace LavaCake {

    /**
     * \brief Rendering attachment configuration for dynamic rendering
     */
    struct RenderingAttachment {
        vk::ImageView m_imageView;                                      ///< The image view to render to
        vk::ImageLayout m_imageLayout = vk::ImageLayout::eColorAttachmentOptimal; ///< The image layout during rendering
        vk::ResolveModeFlagBits m_resolveMode = vk::ResolveModeFlagBits::eNone;   ///< Resolve mode for MSAA
        vk::ImageView m_resolveImageView = nullptr;                     ///< Resolve target for MSAA
        vk::ImageLayout m_resolveImageLayout = vk::ImageLayout::eUndefined;       ///< Layout of resolve image
        vk::AttachmentLoadOp m_loadOp = vk::AttachmentLoadOp::eClear;  ///< Load operation at the start of rendering
        vk::AttachmentStoreOp m_storeOp = vk::AttachmentStoreOp::eStore; ///< Store operation at the end of rendering
        vk::ClearValue m_clearValue = {};                               ///< Clear value for the attachment

        /**
         * \brief Helper to create a color attachment
         * \param view the image view to render to
         * \param clearColor the clear color value (default: black with alpha 1.0)
         * \param load the load operation (default: clear)
         * \return the configured RenderingAttachment
         */
        static RenderingAttachment color(vk::ImageView view,
                                        vk::ClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                                        vk::AttachmentLoadOp load = vk::AttachmentLoadOp::eClear) {
            RenderingAttachment attachment{};
            attachment.m_imageView = view;
            attachment.m_imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            attachment.m_loadOp = load;
            attachment.m_storeOp = vk::AttachmentStoreOp::eStore;
            attachment.m_clearValue.color = clearColor;
            return attachment;
        }

        /**
         * \brief Helper to create a depth attachment
         * \param view the image view to render to
         * \param clearDepth the clear depth value (default: 1.0)
         * \param clearStencil the clear stencil value (default: 0)
         * \param load the load operation (default: clear)
         * \return the configured RenderingAttachment
         */
        static RenderingAttachment depth(vk::ImageView view,
                                        float clearDepth = 1.0f,
                                        uint32_t clearStencil = 0,
                                        vk::AttachmentLoadOp load = vk::AttachmentLoadOp::eClear) {
            RenderingAttachment attachment{};
            attachment.m_imageView = view;
            attachment.m_imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            attachment.m_loadOp = load;
            attachment.m_storeOp = vk::AttachmentStoreOp::eStore;
            attachment.m_clearValue.depthStencil = vk::ClearDepthStencilValue(clearDepth, clearStencil);
            return attachment;
        }

        /**
         * \brief Helper to create a read-only depth attachment
         * \param view the image view to read from
         * \return the configured RenderingAttachment
         */
        static RenderingAttachment depthReadOnly(vk::ImageView view) {
            RenderingAttachment attachment{};
            attachment.m_imageView = view;
            attachment.m_imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            attachment.m_loadOp = vk::AttachmentLoadOp::eLoad;
            attachment.m_storeOp = vk::AttachmentStoreOp::eNone;
            return attachment;
        }
    };

    /**
     * \brief Dynamic rendering context for modern Vulkan rendering without render passes
     */
    class DynamicRenderingContext {
    private:
        vk::Rect2D m_renderArea;                                    ///< The rendering area
        uint32_t m_layerCount = 1;                                  ///< Number of layers for layered rendering
        uint32_t m_viewMask = 0;                                    ///< View mask for multiview rendering

        std::vector<RenderingAttachment> m_colorAttachments;        ///< Color attachments
        std::optional<RenderingAttachment> m_depthAttachment;       ///< Optional depth attachment
        std::optional<RenderingAttachment> m_stencilAttachment;     ///< Optional stencil attachment

        vk::RenderingFlags m_renderingFlags = {};                   ///< Rendering flags (e.g., for local read)

        bool m_isRecording = false;                                 ///< Whether rendering is currently active

    public:
        /**
         * \brief Builder class for constructing DynamicRenderingContext instances
         */
        class Builder {
        private:
            vk::Rect2D m_area = {{0, 0}, {0, 0}};
            uint32_t m_layers = 1;
            uint32_t m_mask = 0;
            
            std::vector<RenderingAttachment> m_colors;
            std::optional<RenderingAttachment> m_depth;
            std::optional<RenderingAttachment> m_stencil;
            vk::RenderingFlags m_flags = {};

        public:
            /**
             * \brief Constructs a Builder for a DynamicRenderingContext
             */
            Builder() {}

            /**
             * \brief Set the render area dimensions and offset
             * \param width the width of the render area
             * \param height the height of the render area
             * \param x the x offset (default: 0)
             * \param y the y offset (default: 0)
             * \return reference to this builder for method chaining
             */
            Builder& setRenderArea(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) {
                m_area.offset = VkOffset2D({x, y});
                m_area.extent = VkExtent2D({width, height});
                return *this;
            }

            /**
             * \brief Set the render area from Vulkan extent and offset
             * \param extent the render area extent
             * \param offset the render area offset (default: {0, 0})
             * \return reference to this builder for method chaining
             */
            Builder& setRenderArea(vk::Extent2D extent, vk::Offset2D offset = {0, 0}) {
                m_area.offset = offset;
                m_area.extent = extent;
                return *this;
            }

            /**
             * \brief Add a color attachment to the rendering context
             * \param attachment the rendering attachment configuration
             * \return reference to this builder for method chaining
             */
            Builder& addColorAttachment(const RenderingAttachment& attachment) {
                m_colors.push_back(attachment);
                return *this;
            }

            /**
             * \brief Add a color attachment with automatic configuration
             * \param view the image view to render to
             * \param clearColor the clear color value (default: black with alpha 1.0)
             * \param loadOp the load operation (default: clear)
             * \return reference to this builder for method chaining
             */
            Builder& addColorAttachment(vk::ImageView view,
                                    vk::ClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                                    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) {
                m_colors.push_back(RenderingAttachment::color(view, clearColor, loadOp));
                return *this;
            }

            /**
             * \brief Set the depth attachment
             * \param attachment the depth attachment configuration
             * \return reference to this builder for method chaining
             */
            Builder& setDepthAttachment(const RenderingAttachment& attachment) {
                m_depth = attachment;
                return *this;
            }

            /**
             * \brief Set the depth attachment with automatic configuration
             * \param view the depth image view
             * \param clearDepth the clear depth value (default: 1.0)
             * \param loadOp the load operation (default: clear)
             * \return reference to this builder for method chaining
             */
            Builder& setDepthAttachment(vk::ImageView view,
                                    float clearDepth = 1.0f,
                                    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) {
                m_depth = RenderingAttachment::depth(view, clearDepth, 0, loadOp);
                return *this;
            }

            /**
             * \brief Set the stencil attachment (usually same as depth)
             * \param attachment the stencil attachment configuration
             * \return reference to this builder for method chaining
             */
            Builder& setStencilAttachment(const RenderingAttachment& attachment) {
                m_stencil = attachment;
                return *this;
            }

            /**
             * \brief Set the view mask for multiview rendering
             * \param viewMask the view mask
             * \return reference to this builder for method chaining
             */
            Builder& setViewMask(uint32_t viewMask) {
                m_mask = viewMask;
                return *this;
            }

            /**
             * \brief Set the layer count for layered rendering (e.g., cubemaps)
             * \param count the number of layers
             * \return reference to this builder for method chaining
             */
            Builder& setLayerCount(uint32_t count) {
                m_layers = count;
                return *this;
            }

            /**
             * \brief Enable local read for deferred rendering
             * \return reference to this builder for method chaining
             */
            Builder& enableLocalRead() {
                m_flags |= vk::RenderingFlagBits::eContentsInlineEXT;
                return *this;
            }

            /**
             * \brief Set resuming flag (continuing from suspended state)
             * \return reference to this builder for method chaining
             */
            Builder& setResuming() {
                m_flags |= vk::RenderingFlagBits::eResuming;
                return *this;
            }

            /**
             * \brief Set suspending flag (can be resumed later)
             * \return reference to this builder for method chaining
             */
            Builder& setSuspending() {
                m_flags |= vk::RenderingFlagBits::eSuspending;
                return *this;
            }

            /**
             * \brief Build and begin the rendering context
             * \param cmd the command buffer to record rendering commands into
             * \return the constructed and begun DynamicRenderingContext
             */
            DynamicRenderingContext begin(vk::CommandBuffer cmd) {
                if (m_area.extent.width == 0 || m_area.extent.height == 0) {
                    throw std::runtime_error("Render area not set or invalid");
                }
                
                DynamicRenderingContext context;
                context.m_renderArea = m_area;
                context.m_layerCount = m_layers;
                context.m_viewMask = m_mask;
                context.m_colorAttachments = m_colors;
                context.m_depthAttachment = m_depth;
                context.m_stencilAttachment = m_stencil;
                context.m_renderingFlags = m_flags;
                
                context.beginRendering(cmd);
                return context;
            }
        };

        /**
         * \brief Default constructor
         */
        DynamicRenderingContext() = default;

        /**
         * \brief End rendering
         * \param commandBuffer the command buffer to end rendering on
         */
        void end(vk::CommandBuffer commandBuffer) {
            if (m_isRecording) {
                commandBuffer.endRendering();
                m_isRecording = false;
            }
        }

        /*
        * we delete const copy and const = operator to avoid duplication
        */
        DynamicRenderingContext(const DynamicRenderingContext&) = delete;
        DynamicRenderingContext& operator=(const DynamicRenderingContext&) = delete;

        /**
         * \brief Move constructor
         * \param other the DynamicRenderingContext to move from
         */
        DynamicRenderingContext(DynamicRenderingContext&& other) noexcept :
            m_renderArea(other.m_renderArea)
            , m_layerCount(other.m_layerCount)
            , m_viewMask(other.m_viewMask)
            , m_colorAttachments(std::move(other.m_colorAttachments))
            , m_depthAttachment(std::move(other.m_depthAttachment))
            , m_stencilAttachment(std::move(other.m_stencilAttachment))
            , m_renderingFlags(other.m_renderingFlags)
            , m_isRecording(other.m_isRecording) {
            other.m_isRecording = false;
        }

        /**
         * \brief Move assignment operator
         * \param other the DynamicRenderingContext to move from
         * \return reference to this DynamicRenderingContext
         */
        DynamicRenderingContext& operator=(DynamicRenderingContext&& other) noexcept {
            if (this != &other) {
                m_renderArea = other.m_renderArea;
                m_layerCount = other.m_layerCount;
                m_viewMask = other.m_viewMask;
                m_colorAttachments = std::move(other.m_colorAttachments);
                m_depthAttachment = std::move(other.m_depthAttachment);
                m_stencilAttachment = std::move(other.m_stencilAttachment);
                m_renderingFlags = other.m_renderingFlags;
                m_isRecording = other.m_isRecording;
                other.m_isRecording = false;
            }
            return *this;
        }

        /**
         * \brief Get the render area
         * \return the vk::Rect2D render area
         */
        vk::Rect2D getRenderArea() const { return m_renderArea; }

        /**
         * \brief Set the viewport
         * \param commandBuffer the command buffer
         * \param width the viewport width
         * \param height the viewport height
         * \param x the viewport x offset (default: 0.0)
         * \param y the viewport y offset (default: 0.0)
         * \param minDepth the minimum depth value (default: 0.0)
         * \param maxDepth the maximum depth value (default: 1.0)
         */
        void setViewport(vk::CommandBuffer commandBuffer, float width, float height, float x = 0.0f, float y = 0.0f,
                        float minDepth = 0.0f, float maxDepth = 1.0f) const {
            vk::Viewport viewport{x, y, width, height, minDepth, maxDepth};
            commandBuffer.setViewport(0, 1, &viewport);
        }

        /**
         * \brief Set the viewport to match the render area
         * \param commandBuffer the command buffer
         */
        void setViewportFromRenderArea(vk::CommandBuffer commandBuffer) const {
            setViewport(
                commandBuffer,
                static_cast<float>(m_renderArea.extent.width),
                static_cast<float>(m_renderArea.extent.height),
                static_cast<float>(m_renderArea.offset.x),
                static_cast<float>(m_renderArea.offset.y)
            );
        }

        /**
         * \brief Set the scissor rectangle
         * \param commandBuffer the command buffer
         * \param width the scissor width
         * \param height the scissor height
         * \param x the scissor x offset (default: 0)
         * \param y the scissor y offset (default: 0)
         */
        void setScissor(vk::CommandBuffer commandBuffer, uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) const {
            vk::Rect2D scissor{{x, y}, {width, height}};
            commandBuffer.setScissor(0, 1, &scissor);
        }

        /**
         * \brief Set the scissor rectangle to match the render area
         * \param commandBuffer the command buffer
         */
        void setScissorFromRenderArea(vk::CommandBuffer commandBuffer) const {
            commandBuffer.setScissor(0, 1, &m_renderArea);
        }

        /**
         * \brief Set viewport and scissor to match the render area (common case)
         * \param commandBuffer the command buffer
         */
        void setDefaultViewportScissor(vk::CommandBuffer commandBuffer) const {
            setViewportFromRenderArea(commandBuffer);
            setScissorFromRenderArea(commandBuffer);
        }

    private:
        /**
         * \brief Begin the rendering pass
         * \param commandBuffer the command buffer to record into
         */
        void beginRendering(vk::CommandBuffer commandBuffer) {
            // Convert attachments to Vulkan structures
            std::vector<vk::RenderingAttachmentInfo> colorInfos;
            for (const auto& attachment : m_colorAttachments) {
                vk::RenderingAttachmentInfo info{};
                info.imageView = attachment.m_imageView;
                info.imageLayout = attachment.m_imageLayout;
                info.resolveMode = attachment.m_resolveMode;
                info.resolveImageView = attachment.m_resolveImageView;
                info.resolveImageLayout = attachment.m_resolveImageLayout;
                info.loadOp = attachment.m_loadOp;
                info.storeOp = attachment.m_storeOp;
                info.clearValue = attachment.m_clearValue;
                colorInfos.push_back(info);
            }
            
            vk::RenderingAttachmentInfo depthInfo{};
            vk::RenderingAttachmentInfo stencilInfo{};
            
            if (m_depthAttachment) {
                depthInfo.imageView = m_depthAttachment->m_imageView;
                depthInfo.imageLayout = m_depthAttachment->m_imageLayout;
                depthInfo.resolveMode = m_depthAttachment->m_resolveMode;
                depthInfo.resolveImageView = m_depthAttachment->m_resolveImageView;
                depthInfo.resolveImageLayout = m_depthAttachment->m_resolveImageLayout;
                depthInfo.loadOp = m_depthAttachment->m_loadOp;
                depthInfo.storeOp = m_depthAttachment->m_storeOp;
                depthInfo.clearValue = m_depthAttachment->m_clearValue;
            }
            
            if (m_stencilAttachment) {
                stencilInfo.imageView = m_stencilAttachment->m_imageView;
                stencilInfo.imageLayout = m_stencilAttachment->m_imageLayout;
                stencilInfo.resolveMode = m_stencilAttachment->m_resolveMode;
                stencilInfo.resolveImageView = m_stencilAttachment->m_resolveImageView;
                stencilInfo.resolveImageLayout = m_stencilAttachment->m_resolveImageLayout;
                stencilInfo.loadOp = m_stencilAttachment->m_loadOp;
                stencilInfo.storeOp = m_stencilAttachment->m_storeOp;
                stencilInfo.clearValue = m_stencilAttachment->m_clearValue;
            }
            
            // Create rendering info
            vk::RenderingInfo renderingInfo{};
            renderingInfo.flags = m_renderingFlags;
            renderingInfo.renderArea = m_renderArea;
            renderingInfo.layerCount = m_layerCount;
            renderingInfo.viewMask = m_viewMask;
            renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorInfos.size());
            renderingInfo.pColorAttachments = colorInfos.empty() ? nullptr : colorInfos.data();
            renderingInfo.pDepthAttachment = m_depthAttachment ? &depthInfo : nullptr;
            renderingInfo.pStencilAttachment = m_stencilAttachment ? &stencilInfo : nullptr;
            
            commandBuffer.beginRendering(renderingInfo);
            m_isRecording = true;
        }
    };

    /**
     * \brief Helper function to begin rendering with common setup
     * \param cmd the command buffer
     * \param colorTarget the color target image view
     * \param extent the render area extent
     * \param clearColor the clear color value (default: black with alpha 1.0)
     * \param depthTarget the optional depth target image view (default: none)
     * \param clearDepth the clear depth value (default: 1.0)
     * \return the begun DynamicRenderingContext
     */
    inline DynamicRenderingContext beginRendering(
        vk::CommandBuffer cmd,
        vk::ImageView colorTarget,
        vk::Extent2D extent,
        vk::ClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
        std::optional<vk::ImageView> depthTarget = std::nullopt,
        float clearDepth = 1.0f) {
        
        auto builder = DynamicRenderingContext::Builder()
            .setRenderArea(extent)
            .addColorAttachment(colorTarget, clearColor);
        
        if (depthTarget) {
            builder.setDepthAttachment(*depthTarget, clearDepth);
        }
        
        return builder.begin(cmd);
    }

    /**
     * \brief Helper function for deferred rendering with multiple render targets
     * \param cmd the command buffer
     * \param extent the render area extent
     * \param albedo the albedo/color G-buffer target
     * \param normal the normal G-buffer target
     * \param materialProps the material properties G-buffer target
     * \param depth the depth buffer target
     * \param clearAttachments whether to clear attachments (default: true)
     * \return the begun DynamicRenderingContext
     */
    inline DynamicRenderingContext beginDeferredRendering(
        vk::CommandBuffer cmd,
        vk::Extent2D extent,
        vk::ImageView albedo,
        vk::ImageView normal,
        vk::ImageView materialProps,
        vk::ImageView depth,
        bool clearAttachments = true) {
        
        auto loadOp = clearAttachments ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
        
        return DynamicRenderingContext::Builder()
            .setRenderArea(extent)
            .addColorAttachment(albedo, {0.0f, 0.0f, 0.0f, 1.0f}, loadOp)
            .addColorAttachment(normal, {0.0f, 0.0f, 0.0f, 0.0f}, loadOp)
            .addColorAttachment(materialProps, {0.0f, 0.0f, 0.0f, 0.0f}, loadOp)
            .setDepthAttachment(depth, 1.0f, loadOp)
            .enableLocalRead()  // Enable for composition pass
            .begin(cmd);
    }
}