#pragma once

#include "Device.hpp"
#include <vector>
#include <memory>
#include <stdexcept>


namespace LavaCake{
// ========== DESCRIPTOR POOL BUILDER ==========
    /**
     * \brief Manages Vulkan descriptor pools for allocating descriptor sets
     */
    class DescriptorPool {
    private:
        LavaCake::Device m_device;
        vk::DescriptorPool m_pool;
        uint32_t m_maxSets;
        vk::DescriptorPoolCreateFlags m_flags;

    public:
        /**
         * \brief Builder class for constructing DescriptorPool instances with various configurations
         */
        class Builder {
        private:
            LavaCake::Device m_device;
            std::vector<vk::DescriptorPoolSize> m_poolSizes;
            uint32_t m_maxSets = 1000;
            vk::DescriptorPoolCreateFlags m_flags;

        public:
            /**
             * \brief Constructs a Builder for a DescriptorPool
             * \param dev the device on which the descriptor pool will be created
             */
            explicit Builder(const LavaCake::Device& dev) : m_device(dev) {}

            /**
             * \brief Set maximum number of descriptor sets that can be allocated from the pool
             * \param count the maximum number of descriptor sets
             * \return reference to this builder for method chaining
             */
            Builder& setMaxSets(uint32_t count) {
                m_maxSets = count;
                return *this;
            }

            /**
             * \brief Add pool size for a specific descriptor type
             * \param type the descriptor type
             * \param count the number of descriptors of this type
             * \return reference to this builder for method chaining
             */
            Builder& addPoolSize(vk::DescriptorType type, uint32_t count) {
                m_poolSizes.push_back({type, count});
                return *this;
            }

            /**
             * \brief Convenience method to add uniform buffer descriptors
             * \param count the number of uniform buffer descriptors
             * \return reference to this builder for method chaining
             */
            Builder& addUniformBuffers(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eUniformBuffer, count);
            }

            /**
             * \brief Convenience method to add storage buffer descriptors
             * \param count the number of storage buffer descriptors
             * \return reference to this builder for method chaining
             */
            Builder& addStorageBuffers(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eStorageBuffer, count);
            }

            /**
             * \brief Convenience method to add combined image sampler descriptors
             * \param count the number of combined image sampler descriptors
             * \return reference to this builder for method chaining
             */
            Builder& addCombinedImageSamplers(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eCombinedImageSampler, count);
            }

            /**
             * \brief Convenience method to add storage image descriptors
             * \param count the number of storage image descriptors
             * \return reference to this builder for method chaining
             */
            Builder& addStorageImages(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eStorageImage, count);
            }

            /**
             * \brief Convenience method to add acceleration structure descriptors for ray tracing
             * \param count the number of acceleration structure descriptors
             * \return reference to this builder for method chaining
             */
            Builder& addAccelerationStructures(uint32_t count) {
                return addPoolSize(vk::DescriptorType::eAccelerationStructureKHR, count);
            }

            /**
             * \brief Enable freeing individual descriptor sets from the pool
             * \return reference to this builder for method chaining
             */
            Builder& setFreeDescriptorSetFlag() {
                m_flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
                return *this;
            }

            /**
             * \brief Enable update after bind flag, required for bindless descriptor patterns
             * \return reference to this builder for method chaining
             */
            Builder& setUpdateAfterBindFlag() {
                m_flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
                return *this;
            }

            /**
             * \brief Apply a preset configuration optimized for bindless rendering
             * \param maxTextures the maximum number of texture descriptors (default: 10000)
             * \param maxBuffers the maximum number of buffer descriptors (default: 5000)
             * \param maxSets the maximum number of descriptor sets (default: 100)
             * \return reference to this builder for method chaining
             */
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

            /**
             * \brief Apply a preset configuration optimized for ray tracing
             * \param maxTextures the maximum number of texture descriptors (default: 10000)
             * \param maxBuffers the maximum number of buffer descriptors (default: 5000)
             * \param maxSets the maximum number of descriptor sets (default: 100)
             * \return reference to this builder for method chaining
             */
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

            /**
             * \brief Build and create the descriptor pool
             * \return the constructed DescriptorPool
             */
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

        /**
         * \brief Constructs a DescriptorPool (typically used by Builder)
         * \param dev the device on which the descriptor pool will be created
         */
        DescriptorPool(const LavaCake::Device& dev) : m_device(dev) {}


        /**
         * \brief Destroys and frees the descriptor pool
         */
        void freeDescriptorPool(){
            if (m_pool) {
                m_device.getDevice().destroyDescriptorPool(m_pool);
            }
        }


        /**
         * \brief Returns the underlying Vulkan descriptor pool
         * \return the vk::DescriptorPool
         */
        vk::DescriptorPool getPool() const { return m_pool; }

        /**
         * \brief Implicit conversion operator to vk::DescriptorPool
         * \return the vk::DescriptorPool
         */
        operator vk::DescriptorPool() const { return m_pool; }

        /**
         * \brief Allocate a single descriptor set from the pool
         * \param layout the descriptor set layout to use
         * \return the allocated vk::DescriptorSet
         */
        vk::DescriptorSet allocate(vk::DescriptorSetLayout layout) {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = m_pool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout;
            
            auto sets = m_device.getDevice().allocateDescriptorSets(allocInfo);
            return sets[0];
        }

        /**
         * \brief Allocate multiple descriptor sets from the pool
         * \param layouts the vector of descriptor set layouts to use
         * \return vector of allocated vk::DescriptorSet
         */
        std::vector<vk::DescriptorSet> allocate(const std::vector<vk::DescriptorSetLayout>& layouts) {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = m_pool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
            allocInfo.pSetLayouts = layouts.data();
            
            return m_device.getDevice().allocateDescriptorSets(allocInfo);
        }

        /**
         * \brief Allocate a descriptor set with variable descriptor count for bindless rendering
         * \param layout the descriptor set layout to use
         * \param variableDescriptorCount the number of variable descriptors to allocate
         * \return the allocated vk::DescriptorSet
         */
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

        /**
         * \brief Free a single descriptor set (only if pool created with FREE_DESCRIPTOR_SET flag)
         * \param set the descriptor set to free
         */
        void free(vk::DescriptorSet set) {
            if (!(m_flags & vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)) {
                throw std::runtime_error("Cannot free descriptor set - pool not created with FREE_DESCRIPTOR_SET flag");
            }
            (void)m_device.getDevice().freeDescriptorSets(m_pool, 1, &set);
        }

        /**
         * \brief Free multiple descriptor sets (only if pool created with FREE_DESCRIPTOR_SET flag)
         * \param sets the vector of descriptor sets to free
         */
        void free(const std::vector<vk::DescriptorSet>& sets) {
            if (!(m_flags & vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)) {
                throw std::runtime_error("Cannot free descriptor sets - pool not created with FREE_DESCRIPTOR_SET flag");
            }
            m_device.getDevice().freeDescriptorSets(m_pool, sets);
        }

        /**
         * \brief Reset the descriptor pool, freeing all allocated descriptor sets
         */
        void reset() {
            m_device.getDevice().resetDescriptorPool(m_pool);
        }
    };

    // ========== COMPLETE DESCRIPTOR SET CREATION HELPER ==========
    /**
     * \brief Helper class for managing descriptor set allocation and layout creation
     */
    class DescriptorSetAllocator {
    private:
        vk::Device device;
        std::unique_ptr<DescriptorPool> pool;

    public:
        /**
         * \brief Constructs a DescriptorSetAllocator
         * \param dev the Vulkan device
         * \param descriptorPool the descriptor pool to use for allocations
         */
        DescriptorSetAllocator(const vk::Device& dev, std::unique_ptr<DescriptorPool> descriptorPool)
            : device(dev), pool(std::move(descriptorPool)) {}

        /**
         * \brief Create a descriptor set layout and allocate a descriptor set in one call
         * \param builder the layout builder to use
         * \return pair containing the unique pointer to the layout and the allocated descriptor set
         */
        template<typename LayoutBuilder>
        std::pair<std::unique_ptr<DescriptorSetLayout>, vk::DescriptorSet>
        createLayoutAndSet(LayoutBuilder&& builder) {
            auto layout = std::forward<LayoutBuilder>(builder).build();
            auto set = pool->allocate(layout->get());
            return {std::move(layout), set};
        }

        /**
         * \brief Allocate a descriptor set from an existing layout
         * \param layout the LavaCake descriptor set layout
         * \return the allocated vk::DescriptorSet
         */
        vk::DescriptorSet allocate(const LavaCake::DescriptorSetLayout& layout) {
            return pool->allocate(layout.getLayout());
        }

        /**
         * \brief Allocate a descriptor set from an existing Vulkan layout
         * \param layout the Vulkan descriptor set layout
         * \return the allocated vk::DescriptorSet
         */
        vk::DescriptorSet allocate(vk::DescriptorSetLayout layout) {
            return pool->allocate(layout);
        }

        /**
         * \brief Get the underlying descriptor pool for manual operations
         * \return reference to the DescriptorPool
         */
        DescriptorPool& getPool() { return *pool; }

        /**
         * \brief Get the underlying descriptor pool for manual operations (const version)
         * \return const reference to the DescriptorPool
         */
        const DescriptorPool& getPool() const { return *pool; }
    };

}