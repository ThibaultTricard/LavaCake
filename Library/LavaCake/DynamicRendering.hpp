#pragma once

#include "Device.hpp"
#include <vector>
#include <optional>
#include <stdexcept>

namespace LavaCake {

    // ========== RENDERING ATTACHMENT ==========
    struct RenderingAttachment {
        vk::ImageView m_imageView;
        vk::ImageLayout m_imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        vk::ResolveModeFlagBits m_resolveMode = vk::ResolveModeFlagBits::eNone;
        vk::ImageView m_resolveImageView = nullptr;
        vk::ImageLayout m_resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp m_loadOp = vk::AttachmentLoadOp::eClear;
        vk::AttachmentStoreOp m_storeOp = vk::AttachmentStoreOp::eStore;
        vk::ClearValue m_clearValue = {};
        
        // Helpers to create common attachment types
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
        
        static RenderingAttachment depthReadOnly(vk::ImageView view) {
            RenderingAttachment attachment{};
            attachment.m_imageView = view;
            attachment.m_imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            attachment.m_loadOp = vk::AttachmentLoadOp::eLoad;
            attachment.m_storeOp = vk::AttachmentStoreOp::eNone;
            return attachment;
        }
    };

    // ========== DYNAMIC RENDERING CONTEXT ==========
    class DynamicRenderingContext {
    private:
        vk::Rect2D m_renderArea;
        uint32_t m_layerCount = 1;
        uint32_t m_viewMask = 0;
        
        std::vector<RenderingAttachment> m_colorAttachments;
        std::optional<RenderingAttachment> m_depthAttachment;
        std::optional<RenderingAttachment> m_stencilAttachment;
        
        // For local read (deferred rendering)
        vk::RenderingFlags m_renderingFlags = {};
        
        bool m_isRecording = false;

    public:
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
            Builder() {}
            
            // Set render area
            Builder& setRenderArea(uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) {
                m_area.offset = VkOffset2D({x, y});
                m_area.extent = VkExtent2D({width, height});
                return *this;
            }
            
            Builder& setRenderArea(vk::Extent2D extent, vk::Offset2D offset = {0, 0}) {
                m_area.offset = offset;
                m_area.extent = extent;
                return *this;
            }
            
            // Add color attachments
            Builder& addColorAttachment(const RenderingAttachment& attachment) {
                m_colors.push_back(attachment);
                return *this;
            }
            
            Builder& addColorAttachment(vk::ImageView view,
                                    vk::ClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                                    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) {
                m_colors.push_back(RenderingAttachment::color(view, clearColor, loadOp));
                return *this;
            }
            
            // Set depth attachment
            Builder& setDepthAttachment(const RenderingAttachment& attachment) {
                m_depth = attachment;
                return *this;
            }
            
            Builder& setDepthAttachment(vk::ImageView view,
                                    float clearDepth = 1.0f,
                                    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear) {
                m_depth = RenderingAttachment::depth(view, clearDepth, 0, loadOp);
                return *this;
            }
            
            // Set stencil attachment (usually same as depth)
            Builder& setStencilAttachment(const RenderingAttachment& attachment) {
                m_stencil = attachment;
                return *this;
            }
            
            // For multiview rendering
            Builder& setViewMask(uint32_t viewMask) {
                m_mask = viewMask;
                return *this;
            }
            
            // For layered rendering (e.g., cubemaps)
            Builder& setLayerCount(uint32_t count) {
                m_layers = count;
                return *this;
            }
            
            // Enable local read for deferred rendering
            Builder& enableLocalRead() {
                m_flags |= vk::RenderingFlagBits::eContentsInlineEXT;
                return *this;
            }
            
            // Resume rendering (continuing from suspended state)
            Builder& setResuming() {
                m_flags |= vk::RenderingFlagBits::eResuming;
                return *this;
            }
            
            // Suspend rendering (can be resumed later)
            Builder& setSuspending() {
                m_flags |= vk::RenderingFlagBits::eSuspending;
                return *this;
            }
            
            // Build and begin rendering
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
        
        DynamicRenderingContext() = default;
        
        // End rendering (called automatically by destructor)
        void end(vk::CommandBuffer commandBuffer) {
            if (m_isRecording) {
                commandBuffer.endRendering();
                m_isRecording = false;
            }
        }
        
        
        // Delete copy
        DynamicRenderingContext(const DynamicRenderingContext&) = delete;
        DynamicRenderingContext& operator=(const DynamicRenderingContext&) = delete;
        
        // Allow move
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
        
        // Get render area
        vk::Rect2D getRenderArea() const { return m_renderArea; }
        
        // Set viewport (helper)
        void setViewport(vk::CommandBuffer commandBuffer, float width, float height, float x = 0.0f, float y = 0.0f,
                        float minDepth = 0.0f, float maxDepth = 1.0f) const {
            vk::Viewport viewport{x, y, width, height, minDepth, maxDepth};
            commandBuffer.setViewport(0, 1, &viewport);
        }
        
        // Set viewport from render area
        void setViewportFromRenderArea(vk::CommandBuffer commandBuffer) const {
            setViewport(
                commandBuffer,
                static_cast<float>(m_renderArea.extent.width),
                static_cast<float>(m_renderArea.extent.height),
                static_cast<float>(m_renderArea.offset.x),
                static_cast<float>(m_renderArea.offset.y)
            );
        }
        
        // Set scissor (helper)
        void setScissor(vk::CommandBuffer commandBuffer, uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) const {
            vk::Rect2D scissor{{x, y}, {width, height}};
            commandBuffer.setScissor(0, 1, &scissor);
        }
        
        // Set scissor from render area
        void setScissorFromRenderArea(vk::CommandBuffer commandBuffer) const {
            commandBuffer.setScissor(0, 1, &m_renderArea);
        }
        
        // Set viewport and scissor from render area (common case)
        void setDefaultViewportScissor(vk::CommandBuffer commandBuffer) const {
            setViewportFromRenderArea(commandBuffer);
            setScissorFromRenderArea(commandBuffer);
        }

    private:
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

    // ========== RENDERING HELPERS ==========

    // Simple helper to begin rendering with common setup
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

    // Helper for deferred rendering with multiple render targets
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