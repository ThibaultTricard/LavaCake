#pragma once

#include "Device.hpp"
#include "Image.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>

namespace LavaCake{

    // ========== DESCRIPTOR SET LAYOUT BUILDER ==========
    /**
     * \brief Manages Vulkan descriptor set layouts
     */
    class DescriptorSetLayout {
    private:
        LavaCake::Device m_device;
        vk::DescriptorSetLayout m_layout;
        
        /**
         * \brief Store binding information for validation
         */
        struct BindingInfo {
            vk::DescriptorType type;
            uint32_t count;
            vk::ShaderStageFlags stages;
            vk::DescriptorBindingFlags flags;
        };
        std::unordered_map<uint32_t, BindingInfo> m_bindings;

    public:
        /**
         * \brief Builder class for constructing DescriptorSetLayout instances
         */
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
            /**
             * \brief Constructs a Builder for a DescriptorSetLayout
             * \param dev the device on which the descriptor set layout will be created
             */
            explicit Builder(LavaCake::Device& dev) : m_device(dev) {}

            /**
             * \brief Add a new descriptor binding to the layout
             * \param binding the binding number
             * \param type the descriptor type
             * \param stages the shader stages that will access this binding
             * \param count the number of descriptors in this binding (default: 1)
             * \return reference to this builder for method chaining
             */
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


            /**
             * \brief Convenience method to add a uniform buffer binding
             * \param binding the binding number
             * \param stages the shader stages that will access this binding
             * \return reference to this builder for method chaining
             */
            Builder& addUniformBuffer(uint32_t binding, vk::ShaderStageFlags stages) {
                return addBinding(binding, vk::DescriptorType::eUniformBuffer, stages, 1);
            }


            /**
             * \brief Convenience method to add a storage buffer binding
             * \param binding the binding number
             * \param stages the shader stages that will access this binding
             * \param count the number of descriptors in this binding (default: 1)
             * \return reference to this builder for method chaining
             */
            Builder& addStorageBuffer(uint32_t binding, vk::ShaderStageFlags stages,
                                    uint32_t count = 1) {
                return addBinding(binding, vk::DescriptorType::eStorageBuffer, stages, count);
            }


            /**
             * \brief Convenience method to add a combined image sampler binding
             * \param binding the binding number
             * \param stages the shader stages that will access this binding
             * \param count the number of descriptors in this binding (default: 1)
             * \return reference to this builder for method chaining
             */
            Builder& addCombinedImageSampler(uint32_t binding, vk::ShaderStageFlags stages,
                                            uint32_t count = 1) {
                return addBinding(binding, vk::DescriptorType::eCombinedImageSampler, 
                                stages, count);
            }


            /**
             * \brief Convenience method to add a storage image binding
             * \param binding the binding number
             * \param stages the shader stages that will access this binding
             * \param count the number of descriptors in this binding (default: 1)
             * \return reference to this builder for method chaining
             */
            Builder& addStorageImage(uint32_t binding, vk::ShaderStageFlags stages,
                                uint32_t count = 1) {
                return addBinding(binding, vk::DescriptorType::eStorageImage, stages, count);
            }


            /**
             * \brief Convenience method to add an acceleration structure binding for ray tracing
             * \param binding the binding number
             * \param stages the shader stages that will access this binding
             * \return reference to this builder for method chaining
             */
            Builder& addAccelerationStructure(uint32_t binding, vk::ShaderStageFlags stages) {
                return addBinding(binding, vk::DescriptorType::eAccelerationStructureKHR,
                                stages, 1);
            }


            /**
             * \brief Set the descriptor count for the current binding
             * \param count the number of descriptors
             * \return reference to this builder for method chaining
             */
            Builder& setCount(uint32_t count) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->count = count;
                return *this;
            }


            //TODO check if bindles can be determined automatiquement 
            /**
             * \brief Enable bindless rendering for the current binding (large array with partial binding)
             * \param maxDescriptors the maximum number of descriptors (default: 10000)
             * \return reference to this builder for method chaining
             */
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


            /**
             * \brief Set variable descriptor count for the current binding (typically used for the last binding)
             * \param maxCount the maximum descriptor count
             * \return reference to this builder for method chaining
             */
            Builder& setVariableDescriptorCount(uint32_t maxCount) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->count = maxCount;
                m_currentBinding->flags |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
                return *this;
            }


            /**
             * \brief Add binding flags to the current binding
             * \param flags the descriptor binding flags to add
             * \return reference to this builder for method chaining
             */
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


            /**
             * \brief Set immutable samplers for the current binding
             * \param samplers the vector of immutable samplers
             * \return reference to this builder for method chaining
             */
            Builder& setImmutableSamplers(const std::vector<vk::Sampler>& samplers) {
                if (!m_currentBinding) {
                    throw std::runtime_error("No binding to configure. Call addBinding first.");
                }
                m_currentBinding->immutableSamplers = samplers;
                m_currentBinding->count = static_cast<uint32_t>(samplers.size());
                return *this;
            }


            /**
             * \brief Add layout-level creation flags
             * \param flags the descriptor set layout creation flags
             * \return reference to this builder for method chaining
             */
            Builder& addLayoutFlags(vk::DescriptorSetLayoutCreateFlags flags) {
                m_layoutFlags |= flags;
                return *this;
            }


            /**
             * \brief Build and create the descriptor set layout
             * \return the constructed DescriptorSetLayout
             */
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

        /**
         * \brief Constructs a DescriptorSetLayout (typically used by Builder)
         * \param dev the device on which the descriptor set layout will be created
         */
        DescriptorSetLayout(const LavaCake::Device& dev) : m_device(dev) {}

        /**
         * \brief Destructor
         */
        ~DescriptorSetLayout() {
            if (m_layout) {
                m_device.getDevice().destroyDescriptorSetLayout(m_layout);
                m_layout = nullptr;
            }
        }

        /*
        * we delete const copy and const = operator to avoid duplication
        */
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        /**
         * \brief Move constructor
         * \param other the DescriptorSetLayout to move from
         */
        DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
            : m_device(other.m_device)
            , m_layout(other.m_layout)
            , m_bindings(std::move(other.m_bindings)) {
            other.m_layout = nullptr;
        }

        /**
         * \brief Move assignment operator
         * \param other the DescriptorSetLayout to move from
         * \return reference to this DescriptorSetLayout
         */
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

        /**
         * \brief Returns the underlying Vulkan descriptor set layout
         * \return the vk::DescriptorSetLayout
         */
        vk::DescriptorSetLayout getLayout() const { return m_layout; }

        /**
         * \brief Returns the underlying Vulkan descriptor set layout
         * \return the vk::DescriptorSetLayout&
         */
        vk::DescriptorSetLayout& getLayout() { return m_layout; }


        /**
         * \brief Implicit conversion operator to vk::DescriptorSetLayout
         * \return the vk::DescriptorSetLayout
         */
        operator vk::DescriptorSetLayout() const { return m_layout; }

        /**
         * \brief Implicit conversion operator to vk::DescriptorSetLayout
         * \return the vk::DescriptorSetLayout&
         */
        operator vk::DescriptorSetLayout&() { return m_layout; }


        /**
         * \brief Implicit conversion operator to const vk::DescriptorSetLayout
         * \return the cosnt vk::DescriptorSetLayout
         */
        operator const vk::DescriptorSetLayout() const { return m_layout; }

        /**
         * \brief Implicit conversion operator to const vk::DescriptorSetLayout&
         * \return the const vk::DescriptorSetLayout&
         */
        operator const vk::DescriptorSetLayout&() const { return m_layout; }


        /**
         * \brief Get binding information for validation
         * \return const reference to the map of binding information
         */
        const std::unordered_map<uint32_t, BindingInfo>& getBindings() const {
            return m_bindings;
        }
    };

    // ========== DESCRIPTOR SET UPDATER ==========
    /**
     * \brief Helper class for updating descriptor sets with resources
     */
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
        /**
         * \brief Constructs a DescriptorSetUpdater
         * \param dev the device
         * \param set the descriptor set to update
         */
        DescriptorSetUpdater(const LavaCake::Device& dev, vk::DescriptorSet set)
            : m_device(dev), m_descriptorSet(set) {}

        /**
         * \brief Bind a single uniform buffer to a binding
         * \param binding the binding number
         * \param buffer the buffer to bind
         * \param offset the offset in the buffer (default: 0)
         * \param range the range of the buffer to bind (default: VK_WHOLE_SIZE)
         * \param arrayElement the array element index (default: 0)
         * \return reference to this updater for method chaining
         */
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

        /**
         * \brief Bind a single storage buffer to a binding
         * \param binding the binding number
         * \param buffer the buffer to bind
         * \param offset the offset in the buffer (default: 0)
         * \param range the range of the buffer to bind (default: VK_WHOLE_SIZE)
         * \param arrayElement the array element index (default: 0)
         * \return reference to this updater for method chaining
         */
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

        /**
         * \brief Bind an array of uniform buffers to a binding
         * \param binding the binding number
         * \param buffers the vector of buffers to bind
         * \param arrayElement the starting array element index (default: 0)
         * \return reference to this updater for method chaining
         */
        DescriptorSetUpdater& bindUniformBufferArray(uint32_t binding,
                                            const std::vector<LavaCake::Buffer>& buffers,
                                            uint32_t arrayElement = 0) {
            m_bufferArrays.emplace_back();
            auto& bufferArray = m_bufferArrays.back();
            
            for (const auto& buffer : buffers) {
                bufferArray.push_back({buffer.getBuffer(), 0, VK_WHOLE_SIZE});
            }
            
            vk::WriteDescriptorSet write{};
            write.dstSet = m_descriptorSet;
            write.dstBinding = binding;
            write.dstArrayElement = arrayElement;
            write.descriptorType = vk::DescriptorType::eUniformBuffer; // ← Different type
            write.descriptorCount = static_cast<uint32_t>(bufferArray.size());
            
            UpdatePointerInfos updateInfos;
            updateInfos.type = bindType::eBufferArray;
            updateInfos.listIndex = m_bufferArrays.size()-1;
            m_updatePointerInfos.push_back(updateInfos);
            
            m_writes.push_back(write);
            return *this;
        }

        /**
         * \brief Bind an array of storage buffers to a binding (for bindless rendering)
         * \param binding the binding number
         * \param buffers the vector of buffers to bind
         * \param arrayElement the starting array element index (default: 0)
         * \return reference to this updater for method chaining
         */
        DescriptorSetUpdater& bindStorageBufferArray(uint32_t binding,
                                                    const std::vector<LavaCake::Buffer>& buffers,
                                                    uint32_t arrayElement = 0) {
            m_bufferArrays.emplace_back();
            auto& bufferArray = m_bufferArrays.back();
            
            for (const auto& buffer : buffers) {
                bufferArray.push_back({buffer.getBuffer(), 0, VK_WHOLE_SIZE});
            }
            
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

        /**
         * \brief Bind a single combined image sampler to a binding
         * \param binding the binding number
         * \param imageView the image view to bind
         * \param sampler the sampler to use
         * \param layout the image layout (default: eShaderReadOnlyOptimal)
         * \param arrayElement the array element index (default: 0)
         * \return reference to this updater for method chaining
         */
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

        /**
         * \brief Bind a storage image to a binding
         * \param binding the binding number
         * \param imageView the image view to bind
         * \param layout the image layout (default: eGeneral)
         * \param arrayElement the array element index (default: 0)
         * \return reference to this updater for method chaining
         */
        DescriptorSetUpdater& bindStorageImage(uint32_t binding,
                                            vk::ImageView& imageView,
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

        /**
         * \brief Bind an array of images to a binding (for bindless textures)
         * \param binding the binding number
         * \param imageViews the vector of image views to bind
         * \param sampler the sampler to use for all images
         * \param layout the image layout (default: eShaderReadOnlyOptimal)
         * \param arrayElement the starting array element index (default: 0)
         * \return reference to this updater for method chaining
         */
        DescriptorSetUpdater& bindImageArray(uint32_t binding,
                                            const std::vector<LavaCake::ImageView>& imageViews,
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

        /**
         * \brief Bind individual image to array slot (useful for sparse bindless updates)
         * \param binding the binding number
         * \param arrayIndex the array index
         * \param imageView the image view to bind
         * \param sampler the sampler to use
         * \param layout the image layout
         * \return reference to this updater for method chaining
         */
        /*DescriptorSetUpdater& bindImageToArraySlot(uint32_t binding,
                                                uint32_t arrayIndex,
                                                vk::ImageView imageView,
                                                vk::Sampler sampler,
                                                vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) {
            return bindImage(binding, imageView, sampler, layout, arrayIndex);
        }*/

        /**
         * \brief Bind an acceleration structure for ray tracing
         * \param binding the binding number
         * \param as the acceleration structure to bind
         * \param arrayElement the array element index (default: 0)
         * \return reference to this updater for method chaining
         */
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

        /**
         * \brief Update all bindings and submit to the device
         */
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

        /**
         * \brief Get the number of pending write operations (for debugging)
         * \return the number of write operations
         */
        size_t getWriteCount() const { return m_writes.size(); }
    };

}