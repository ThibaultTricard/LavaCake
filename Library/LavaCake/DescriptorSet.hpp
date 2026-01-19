#pragma once

#include "Device.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>

namespace LavaCake{

    // ========== DESCRIPTOR SET LAYOUT BUILDER ==========
    class DescriptorSetLayout {
    private:
        LavaCake::Device m_device;
        vk::DescriptorSetLayout m_layout;
        
        // Store binding information for validation
        struct BindingInfo {
            vk::DescriptorType type;
            uint32_t count;
            vk::ShaderStageFlags stages;
            vk::DescriptorBindingFlags flags;
        };
        std::unordered_map<uint32_t, BindingInfo> m_bindings;

    public:
        class Builder {
        private:
            LavaCake::Device m_device;
            
            struct BindingConfig {
                uint32_t binding;
                vk::DescriptorType type;
                uint32_t count;
                vk::ShaderStageFlags stages;
                vk::DescriptorBindingFlags flags;
                std::vector<vk::Sampler> immutableSamplers;
            };
            
            std::vector<BindingConfig> m_bindingConfigs;
            vk::DescriptorSetLayoutCreateFlags m_layoutFlags;
            
            // Track current binding being configured
            BindingConfig* m_currentBinding = nullptr;

        public:
            explicit Builder(LavaCake::Device& dev) : m_device(dev) {}
            
            // Start a new binding
            Builder& addBinding(uint32_t binding, 
                            vk::DescriptorType type,
                            vk::ShaderStageFlags stages,
                            uint32_t count = 1) {
                // Check for duplicate bindings
                for (const auto& cfg : m_bindingConfigs) {
                    if (cfg.binding == binding) {
                        throw std::runtime_error("Binding " + std::to_string(binding) + 
                                            " already exists");
                    }
                }
                
                m_bindingConfigs.push_back({binding, type, count, stages, {}, {}});
                m_currentBinding = &m_bindingConfigs.back();
                return *this;
            }
            
            // Convenience methods for common descriptor types
            Builder& addUniformBuffer(uint32_t binding, vk::ShaderStageFlags stages) {
                return addBinding(binding, vk::DescriptorType::eUniformBuffer, stages, 1);
            }
            
            Builder& addStorageBuffer(uint32_t binding, vk::ShaderStageFlags stages, 
                                    uint32_t count = 1) {
                return addBinding(binding, vk::DescriptorType::eStorageBuffer, stages, count);
            }
            
            Builder& addCombinedImageSampler(uint32_t binding, vk::ShaderStageFlags stages,
                                            uint32_t count = 1) {
                return addBinding(binding, vk::DescriptorType::eCombinedImageSampler, 
                                stages, count);
            }
            
            Builder& addStorageImage(uint32_t binding, vk::ShaderStageFlags stages,
                                uint32_t count = 1) {
                return addBinding(binding, vk::DescriptorType::eStorageImage, stages, count);
            }
            
            Builder& addAccelerationStructure(uint32_t binding, vk::ShaderStageFlags stages) {
                return addBinding(binding, vk::DescriptorType::eAccelerationStructureKHR,
                                stages, 1);
            }
            
            // Configure the current binding
            Builder& setCount(uint32_t count) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->count = count;
                return *this;
            }
            
            // Enable bindless for current binding (large array with partial binding)
            Builder& setBindless(uint32_t maxDescriptors = 10000) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->count = maxDescriptors;
                m_currentBinding->flags |= vk::DescriptorBindingFlagBits::ePartiallyBound |
                                        vk::DescriptorBindingFlagBits::eUpdateAfterBind;
                
                // Ensure layout has update-after-bind flag
                m_layoutFlags |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
                return *this;
            }
            
            // Set variable descriptor count (for last binding)
            Builder& setVariableDescriptorCount(uint32_t maxCount) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->count = maxCount;
                m_currentBinding->flags |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
                return *this;
            }
            
            // Add individual binding flags
            Builder& addBindingFlags(vk::DescriptorBindingFlags flags) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->flags |= flags;
                
                // Auto-enable layout flag if needed
                if (flags & vk::DescriptorBindingFlagBits::eUpdateAfterBind) {
                    m_layoutFlags |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
                }
                return *this;
            }
            
            // Set immutable samplers
            Builder& setImmutableSamplers(const std::vector<vk::Sampler>& samplers) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->immutableSamplers = samplers;
                m_currentBinding->count = static_cast<uint32_t>(samplers.size());
                return *this;
            }
            
            // Add layout-level flags
            Builder& addLayoutFlags(vk::DescriptorSetLayoutCreateFlags flags) {
                m_layoutFlags |= flags;
                return *this;
            }
            
            // Build the layout
            DescriptorSetLayout build() {
                if (m_bindingConfigs.empty()) {
                    throw std::runtime_error("Cannot create descriptor set layout with no bindings");
                }
                
                auto result = DescriptorSetLayout(m_device);
                
                // Sort bindings by binding number
                std::sort(m_bindingConfigs.begin(), m_bindingConfigs.end(),
                        [](const BindingConfig& a, const BindingConfig& b) {
                            return a.binding < b.binding;
                        });
                
                // Create Vulkan bindings
                std::vector<vk::DescriptorSetLayoutBinding> bindings;
                std::vector<vk::DescriptorBindingFlags> bindingFlags;
                
                for (const auto& cfg : m_bindingConfigs) {
                    vk::DescriptorSetLayoutBinding binding{};
                    binding.binding = cfg.binding;
                    binding.descriptorType = cfg.type;
                    binding.descriptorCount = cfg.count;
                    binding.stageFlags = cfg.stages;
                    binding.pImmutableSamplers = cfg.immutableSamplers.empty() ? 
                        nullptr : cfg.immutableSamplers.data();
                    
                    bindings.push_back(binding);
                    bindingFlags.push_back(cfg.flags);
                    
                    // Store binding info for validation
                    result.m_bindings[cfg.binding] = {
                        cfg.type, cfg.count, cfg.stages, cfg.flags
                    };
                }
                
                // Create binding flags info
                vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
                bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                bindingFlagsInfo.pBindingFlags = bindingFlags.data();
                
                // Create layout
                vk::DescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.flags = m_layoutFlags;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();
                layoutInfo.pNext = &bindingFlagsInfo;
                
                result.m_layout = m_device.getDevice().createDescriptorSetLayout(layoutInfo);
                
                return result;
            }
        };
        
        DescriptorSetLayout(const LavaCake::Device& dev) : m_device(dev) {}
        
        ~DescriptorSetLayout() {
            if (m_layout) {
                //m_device.getDevice().destroyDescriptorSetLayout(m_layout);
            }
        }
        
        // Delete copy
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
        
        // Allow move
        DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
            : m_device(other.m_device)
            , m_layout(other.m_layout)
            , m_bindings(std::move(other.m_bindings)) {
            other.m_layout = nullptr;
        }
        
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept {
            if (this != &other) {
                if (m_layout) {
                    m_device.getDevice().destroyDescriptorSetLayout(m_layout);
                }
                m_device = other.m_device;
                m_layout = other.m_layout;
                m_bindings = std::move(other.m_bindings);
                other.m_layout = nullptr;
            }
            return *this;
        }
        
        vk::DescriptorSetLayout getLayout() const { return m_layout; }
        operator vk::DescriptorSetLayout() const { return m_layout; }
        
        // Get binding info for validation
        const std::unordered_map<uint32_t, BindingInfo>& getBindings() const {
            return m_bindings;
        }
    };

    // ========== DESCRIPTOR SET UPDATER ==========
    class DescriptorSetUpdater {
    private:
        LavaCake::Device m_device;
        vk::DescriptorSet m_descriptorSet;
        
        std::vector<vk::WriteDescriptorSet> m_writes;
        std::vector<vk::DescriptorBufferInfo> m_bufferInfos;
        std::vector<vk::DescriptorImageInfo> m_imageInfos;
        std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> m_asInfos;
        
        // Storage for arrays
        std::vector<std::vector<vk::DescriptorBufferInfo>> m_bufferArrays;
        std::vector<std::vector<vk::DescriptorImageInfo>> m_imageArrays;

        enum class bindType{
            eBuffer,
            eImage,
            eAccelerationStructure,
            eBufferArray,
            eImageArray
        };

        struct UpdatePointerInfos{
            bindType type;
            uint32_t listIndex;
        };

        std::vector<UpdatePointerInfos> m_updatePointerInfos= {};

    public:
        DescriptorSetUpdater(const LavaCake::Device& dev, vk::DescriptorSet set)
            : m_device(dev), m_descriptorSet(set) {}
        
        // Bind single uniform buffer
        DescriptorSetUpdater& bindUniformBuffer(uint32_t binding,
                                            vk::Buffer buffer,
                                            vk::DeviceSize offset = 0,
                                            vk::DeviceSize range = VK_WHOLE_SIZE,
                                            uint32_t arrayElement = 0) {
            m_bufferInfos.push_back({buffer, offset, range});
            
            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eUniformBuffer;
            write.descriptorCount = 1;
            //write.pBufferInfo = &m_bufferInfos.back();
            
            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eBuffer;
            updateInfos.listIndex = m_bufferInfos.size()-1;
            m_updatePointerInfos.push_back(updateInfos);

            m_writes.push_back(write);
            return *this;
        }
        
        // Bind single storage buffer
        DescriptorSetUpdater& bindStorageBuffer(uint32_t binding,
                                            vk::Buffer buffer,
                                            vk::DeviceSize offset = 0,
                                            vk::DeviceSize range = VK_WHOLE_SIZE,
                                            uint32_t arrayElement = 0) {
                                                
            m_bufferInfos.push_back({buffer, offset, range});
            
            

            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eStorageBuffer;
            write.descriptorCount = 1;
            //write.pBufferInfo = &m_bufferInfos.back();

            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eBuffer;
            updateInfos.listIndex = m_bufferInfos.size()-1;
            m_updatePointerInfos.push_back(updateInfos);
            
            m_writes.push_back(write);

            

            return *this;
        }
        
        // Bind buffer array (for bindless)
        DescriptorSetUpdater& bindStorageBufferArray(uint32_t binding,
                                                    const std::vector<LavaCake::Buffer>& buffers,
                                                    uint32_t arrayElement = 0) {
            m_bufferArrays.emplace_back();
            auto& bufferArray = m_bufferArrays.back();
            
            
            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eStorageBuffer;
            write.descriptorCount = static_cast<uint32_t>(bufferArray.size());
            //write.pBufferInfo = bufferArray.data();

            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eBufferArray;
            updateInfos.listIndex = m_bufferArrays.size()-1;
            m_updatePointerInfos.push_back(updateInfos);
            
            m_writes.push_back(write);
            return *this;
        }
        
        // Bind single image
        DescriptorSetUpdater& bindImage(uint32_t binding,
                                    vk::ImageView imageView,
                                    vk::Sampler sampler,
                                    vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal,
                                    uint32_t arrayElement = 0) {
            m_imageInfos.push_back({sampler, imageView, layout});
            
            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            write.descriptorCount = 1;
            //write.pImageInfo = &m_imageInfos.back();

            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eImage;
            updateInfos.listIndex = m_imageInfos.size()-1;
            m_updatePointerInfos.push_back(updateInfos);
            
            m_writes.push_back(write);
            return *this;
        }
        
        // Bind storage image
        DescriptorSetUpdater& bindStorageImage(uint32_t binding,
                                            vk::ImageView imageView,
                                            vk::ImageLayout layout = vk::ImageLayout::eGeneral,
                                            uint32_t arrayElement = 0) {
            m_imageInfos.push_back({nullptr, imageView, layout});
            
            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eStorageImage;
            write.descriptorCount = 1;
            //write.pImageInfo = &m_imageInfos.back();

            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eImage;
            updateInfos.listIndex = m_imageInfos.size()-1;
            m_updatePointerInfos.push_back(updateInfos);
            
            m_writes.push_back(write);
            return *this;
        }
        
        // Bind image array (for bindless textures)
        DescriptorSetUpdater& bindImageArray(uint32_t binding,
                                            const std::vector<vk::ImageView>& imageViews,
                                            vk::Sampler sampler,
                                            vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal,
                                            uint32_t arrayElement = 0) {
            m_imageArrays.emplace_back();
            auto& imageArray = m_imageArrays.back();
            
            for (const auto& view : imageViews) {
                imageArray.push_back({sampler, view, layout});
            }
            
            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            write.descriptorCount = static_cast<uint32_t>(imageArray.size());
            //write.pImageInfo = imageArray.data();

            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eImageArray;
            updateInfos.listIndex = m_imageArrays.size()-1;
            m_updatePointerInfos.push_back(updateInfos);
            
            m_writes.push_back(write);
            return *this;
        }
        
        // Bind individual image to array slot (useful for sparse bindless updates)
        /*DescriptorSetUpdater& bindImageToArraySlot(uint32_t binding,
                                                uint32_t arrayIndex,
                                                vk::ImageView imageView,
                                                vk::Sampler sampler,
                                                vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) {
            return bindImage(binding, imageView, sampler, layout, arrayIndex);
        }*/
        
        // Bind acceleration structure
        DescriptorSetUpdater& bindAccelerationStructure(uint32_t binding,
                                                    vk::AccelerationStructureKHR as,
                                                    uint32_t arrayElement = 0) {
            m_asInfos.emplace_back();
            auto& asInfo = m_asInfos.back();
            asInfo.accelerationStructureCount = 1;
            asInfo.pAccelerationStructures = new vk::AccelerationStructureKHR(as);
            
            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eAccelerationStructureKHR;
            write.descriptorCount = 1;
            //write.pNext = &asInfo;

            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eAccelerationStructure;
            updateInfos.listIndex = m_asInfos.size()-1;
            m_updatePointerInfos.push_back(updateInfos);
            
            m_writes.push_back(write);
            return *this;
        }
        
        // Update all bindings
        void update() {
            
            for(uint32_t i = 0; i < m_updatePointerInfos.size(); i ++){
                auto u = m_updatePointerInfos[i];

                switch (u.type)
                {
                case bindType::eBuffer:
                    m_writes[i].pBufferInfo = &m_bufferInfos[u.listIndex];
                    break;
                case bindType::eImage:
                    m_writes[i].pImageInfo = &m_imageInfos[u.listIndex];
                    break;
                case bindType::eAccelerationStructure:
                    m_writes[i].pNext = &m_asInfos[u.listIndex];
                    break;
                case bindType::eBufferArray:
                    m_writes[i].pBufferInfo = m_bufferArrays[u.listIndex].data();
                    break;
                case bindType::eImageArray:
                    m_writes[i].pImageInfo = m_imageArrays[u.listIndex].data();
                    break;
                
                default:
                    break;
                }
            }


            if (!m_writes.empty()) {
                m_device.getDevice().updateDescriptorSets(m_writes, {});
            }
            
            // Cleanup AS pointers
            for (auto& asInfo : m_asInfos) {
                delete asInfo.pAccelerationStructures;
            }
        }
        
        // Get write count (for debugging)
        size_t getWriteCount() const { return m_writes.size(); }
    };

}