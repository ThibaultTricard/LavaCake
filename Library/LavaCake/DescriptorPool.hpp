#pragma once

#include "Device.hpp"
#include <vector>
#include <memory>
#include <stdexcept>


namespace LavaCake{
// ========== DESCRIPTOR POOL BUILDER ==========
    class DescriptorPool {
    private:
        LavaCake::Device m_device;
        vk::DescriptorPool m_pool;
        uint32_t m_maxSets;
        vk::DescriptorPoolCreateFlags m_flags;

    public:
        class Builder {
        private:
            LavaCake::Device m_device;
            std::vector<vk::DescriptorPoolSize> m_poolSizes;
            uint32_t m_maxSets = 1000;
            vk::DescriptorPoolCreateFlags m_flags;

        public:
            explicit Builder(const LavaCake::Device& dev) : m_device(dev) {}
            
            // Set maximum number of descriptor sets
            Builder& setMaxSets(uint32_t count) {
                m_maxSets = count;
                return *this;
            }
            
            // Add pool size for a descriptor type
            Builder& addPoolSize(vk::DescriptorType type, uint32_t count) {
                m_poolSizes.push_back({type, count});
                return *this;
            }
            
            // Convenience methods for common types
            Builder& addUniformBuffers(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eUniformBuffer, count);
            }
            
            Builder& addStorageBuffers(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eStorageBuffer, count);
            }
            
            Builder& addCombinedImageSamplers(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eCombinedImageSampler, count);
            }
            
            Builder& addStorageImages(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eStorageImage, count);
            }
            
            Builder& addAccelerationStructures(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eAccelerationStructureKHR, count);
            }
            
            // Enable free individual descriptor sets
            Builder& setFreeDescriptorSetFlag() {
                m_flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
                return *this;
            }
            
            // Enable update after bind (required for bindless)
            Builder& setUpdateAfterBindFlag() {
                m_flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
                return *this;
            }
            
            // Preset for bindless rendering
            Builder& setBindlessPreset(uint32_t maxTextures = 10000,
                                    uint32_t maxBuffers = 5000,
                                    uint32_t maxSets = 100) {
                this->m_maxSets = maxSets;
                m_poolSizes.clear();
                
                addCombinedImageSamplers(maxTextures);
                addStorageBuffers(maxBuffers);
                addUniformBuffers(100);
                addStorageImages(100);
                setUpdateAfterBindFlag();
                
                return *this;
            }
            
            // Preset for ray tracing
            Builder& setRayTracingPreset(uint32_t maxTextures = 10000,
                                        uint32_t maxBuffers = 5000,
                                        uint32_t maxSets = 100) {
                this->m_maxSets = maxSets;
                m_poolSizes.clear();
                
                addCombinedImageSamplers(maxTextures);
                addStorageBuffers(maxBuffers);
                addStorageImages(100);
                addAccelerationStructures(10);
                addUniformBuffers(100);
                setUpdateAfterBindFlag();
                
                return *this;
            }
            
            // Build the pool
            DescriptorPool build() {
                if (m_poolSizes.empty()) {
                    throw std::runtime_error("Cannot create descriptor pool with no pool sizes");
                }
                
                auto result = DescriptorPool(m_device);
                result.m_maxSets = m_maxSets;
                result.m_flags = m_flags;
                
                vk::DescriptorPoolCreateInfo poolInfo{};
                poolInfo.flags = m_flags;
                poolInfo.maxSets = m_maxSets;
                poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
                poolInfo.pPoolSizes = m_poolSizes.data();
                
                result.m_pool = m_device.getDevice().createDescriptorPool(poolInfo);
                
                return result;
            }
        };
        
        DescriptorPool(const LavaCake::Device& dev) : m_device(dev) {}
        

        void freeDescriptorPool(){
            if (m_pool) {
                m_device.getDevice().destroyDescriptorPool(m_pool);
            }
        }
        
        
        vk::DescriptorPool getPool() const { return m_pool; }
        operator vk::DescriptorPool() const { return m_pool; }
        
        // Allocate a single descriptor set
        vk::DescriptorSet allocate(vk::DescriptorSetLayout layout) {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = m_pool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout;
            
            auto sets = m_device.getDevice().allocateDescriptorSets(allocInfo);
            return sets[0];
        }
        
        // Allocate multiple descriptor sets
        std::vector<vk::DescriptorSet> allocate(const std::vector<vk::DescriptorSetLayout>& layouts) {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = m_pool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
            allocInfo.pSetLayouts = layouts.data();
            
            return m_device.getDevice().allocateDescriptorSets(allocInfo);
        }
        
        // Allocate with variable descriptor count (for bindless)
        vk::DescriptorSet allocateVariable(vk::DescriptorSetLayout layout, 
                                        uint32_t variableDescriptorCount) {
            vk::DescriptorSetVariableDescriptorCountAllocateInfo variableInfo{};
            variableInfo.descriptorSetCount = 1;
            variableInfo.pDescriptorCounts = &variableDescriptorCount;
            
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = m_pool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout;
            allocInfo.pNext = &variableInfo;
            
            auto sets = m_device.getDevice().allocateDescriptorSets(allocInfo);
            return sets[0];
        }
        
        // Free a descriptor set (only if pool created with FREE_DESCRIPTOR_SET flag)
        void free(vk::DescriptorSet set) {
            if (!(m_flags & vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)) {
                throw std::runtime_error("Cannot free descriptor set - pool not created with FREE_DESCRIPTOR_SET flag");
            }
            m_device.getDevice().freeDescriptorSets(m_pool, 1, &set);
        }
        
        // Free multiple descriptor sets
        void free(const std::vector<vk::DescriptorSet>& sets) {
            if (!(m_flags & vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)) {
                throw std::runtime_error("Cannot free descriptor sets - pool not created with FREE_DESCRIPTOR_SET flag");
            }
            m_device.getDevice().freeDescriptorSets(m_pool, sets);
        }
        
        // Reset the pool (frees all sets)
        void reset() {
            m_device.getDevice().resetDescriptorPool(m_pool);
        }
    };

    // ========== COMPLETE DESCRIPTOR SET CREATION HELPER ==========
    class DescriptorSetAllocator {
    private:
        vk::Device device;
        std::unique_ptr<DescriptorPool> pool;

    public:
        DescriptorSetAllocator(const vk::Device& dev, std::unique_ptr<DescriptorPool> descriptorPool)
            : device(dev), pool(std::move(descriptorPool)) {}
        
        // Create layout and allocate set in one call
        template<typename LayoutBuilder>
        std::pair<std::unique_ptr<DescriptorSetLayout>, vk::DescriptorSet>
        createLayoutAndSet(LayoutBuilder&& builder) {
            auto layout = std::forward<LayoutBuilder>(builder).build();
            auto set = pool->allocate(layout->get());
            return {std::move(layout), set};
        }
        
        // Allocate from existing layout
        vk::DescriptorSet allocate(const LavaCake::DescriptorSetLayout& layout) {
            return pool->allocate(layout.getLayout());
        }
        
        vk::DescriptorSet allocate(vk::DescriptorSetLayout layout) {
            return pool->allocate(layout);
        }
        
        // Get pool for manual operations
        DescriptorPool& getPool() { return *pool; }
        const DescriptorPool& getPool() const { return *pool; }
    };

}