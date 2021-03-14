#ifdef RAYTRACING
#include "RayTracingPipeline.h"

namespace LavaCake {
	namespace RayTracing {



			void RayTracingPipeline::addRayGenModule(RayGenShaderModule* module) {
				if (m_isHitGroupOpen)
				{
					Framework::ErrorCheck::setError((char*)"Cannot add raygen stage in when hit group open");
					return;
				}

				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
				stageCreate.module = module->getStageParameter().ShaderModule;
				// This member has to be 'main', regardless of the actual entry point of the shader
				stageCreate.pName = "main";
				stageCreate.flags = 0;
				stageCreate.pSpecializationInfo = nullptr;

				m_shaderStages.emplace_back(stageCreate);
				uint32_t shaderIndex = static_cast<uint32_t>(m_shaderStages.size() - 1);

				VkRayTracingShaderGroupCreateInfoKHR groupInfo;
				groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
				groupInfo.pNext = nullptr;
				groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				groupInfo.generalShader = shaderIndex;
				groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
				groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
				groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
				m_shaderGroups.emplace_back(groupInfo);

				m_ShaderBindingTable.addRayGeneration(m_currentGroupIndex, {});
				m_currentGroupIndex++;

			}

			void RayTracingPipeline::addMissModule(MissShaderModule* module) {
				if (m_isHitGroupOpen)
				{
					Framework::ErrorCheck::setError((char*)"Cannot add miss stage in when hit group open");
					return;
				}

				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
				stageCreate.module = module->getStageParameter().ShaderModule;
				// This member has to be 'main', regardless of the actual entry point of the shader
				stageCreate.pName = "main";
				stageCreate.flags = 0;
				stageCreate.pSpecializationInfo = nullptr;

				m_shaderStages.emplace_back(stageCreate);
				uint32_t shaderIndex = static_cast<uint32_t>(m_shaderStages.size() - 1);

				VkRayTracingShaderGroupCreateInfoKHR groupInfo;
				groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
				groupInfo.pNext = nullptr;
				groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				groupInfo.generalShader = shaderIndex;
				groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
				groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
				groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
				m_shaderGroups.emplace_back(groupInfo);
				m_ShaderBindingTable.addMissProgram(m_currentGroupIndex, {});
				m_currentGroupIndex++;
			}

			void RayTracingPipeline::startHitGroup() {
				if (m_isHitGroupOpen)
				{
					Framework::ErrorCheck::setError((char*)"Hit group already open");
					return;
				}

				VkRayTracingShaderGroupCreateInfoKHR groupInfo;
				groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
				groupInfo.pNext = nullptr;
				groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				groupInfo.generalShader = VK_SHADER_UNUSED_KHR;
				groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
				groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
				groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
				m_shaderGroups.push_back(groupInfo);

				m_isHitGroupOpen = true;

			}

			void RayTracingPipeline::setClosestHitModule(ClosestHitShaderModule* module) {
				if (!m_isHitGroupOpen) {
					Framework::ErrorCheck::setError((char*)"No open hitgroup");
					return;
				}
				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage = module->getStageParameter().ShaderStage;
				stageCreate.module = module->getStageParameter().ShaderModule;
				// This member has to be 'main', regardless of the actual entry point of the shader
				stageCreate.pName = module->getStageParameter().EntryPointName;
				stageCreate.flags = 0;
				stageCreate.pSpecializationInfo = nullptr;

				m_shaderStages.emplace_back(stageCreate);
				uint32_t shaderIndex = static_cast<uint32_t>(m_shaderStages.size() - 1);
				m_shaderGroups[m_shaderGroups.size() - 1].closestHitShader = shaderIndex;
			}

			void RayTracingPipeline::setAnyHitModule(AnyHitShaderModule* module) {
				if (!m_isHitGroupOpen) {
					Framework::ErrorCheck::setError((char*)"No open hitgroup");
					return;
				}
				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage = module->getStageParameter().ShaderStage;
				stageCreate.module = module->getStageParameter().ShaderModule;
				// This member has to be 'main', regardless of the actual entry point of the shader
				stageCreate.pName = module->getStageParameter().EntryPointName;
				stageCreate.flags = 0;
				stageCreate.pSpecializationInfo = nullptr;

				m_shaderStages.emplace_back(stageCreate);
				uint32_t shaderIndex = static_cast<uint32_t>(m_shaderStages.size() - 1);
				m_shaderGroups[m_shaderGroups.size() - 1].anyHitShader = shaderIndex;
			}

			void RayTracingPipeline::setIntersectionModule(IntersectionShaderModule* module) {
				if (m_isHitGroupOpen) {
					Framework::ErrorCheck::setError((char*)"No open hitgroup");
					return;
				}
				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage = module->getStageParameter().ShaderStage;
				stageCreate.module = module->getStageParameter().ShaderModule;
				// This member has to be 'main', regardless of the actual entry point of the shader
				stageCreate.pName = module->getStageParameter().EntryPointName;
				stageCreate.flags = 0;
				stageCreate.pSpecializationInfo = nullptr;

				m_shaderStages.emplace_back(stageCreate);
				uint32_t shaderIndex = static_cast<uint32_t>(m_shaderStages.size() - 1);
				m_shaderGroups[m_shaderGroups.size() - 1].intersectionShader = shaderIndex;
			}

			void RayTracingPipeline::endHitGroup() {
				if (!m_isHitGroupOpen)
				{
					Framework::ErrorCheck::setError((char*)"No hit group open");
					return;
				}
				m_isHitGroupOpen = false;
				m_ShaderBindingTable.addHitGroup(m_currentGroupIndex, {});
				m_currentGroupIndex++;
			}

			void RayTracingPipeline::compile(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff) {

				Framework::Device* d = Framework::Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				generateDescriptorLayout();
				InitVkDestroyer(logical, m_pipelineLayout);
				if (!CreatePipelineLayout(logical, { *m_descriptorSetLayout }, {}, *m_pipelineLayout)) {
					Framework::ErrorCheck::setError((char*)"Can't create compute pipeline layout");
				}

				VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{};
				rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
				rayPipelineInfo.pNext = nullptr;
				rayPipelineInfo.flags = 0;
				rayPipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
				rayPipelineInfo.pStages = m_shaderStages.data();
				rayPipelineInfo.groupCount = static_cast<uint32_t>(m_shaderGroups.size());
				rayPipelineInfo.pGroups = m_shaderGroups.data();
				rayPipelineInfo.maxPipelineRayRecursionDepth = m_maxRecursion;
				rayPipelineInfo.layout = *m_pipelineLayout;
				rayPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
				rayPipelineInfo.basePipelineIndex = 0;

				std::vector<VkRayTracingPipelineCreateInfoKHR> pipelineInfos = { rayPipelineInfo };

				InitVkDestroyer(logical, m_pipeline);
				std::vector<VkPipeline> pipelines(pipelineInfos.size());
				VkResult code = vkCreateRayTracingPipelinesKHR(logical, nullptr, nullptr, (uint32_t)pipelineInfos.size(), pipelineInfos.data(), nullptr, pipelines.data());
				*m_pipeline = pipelines[0];



				if (code != VK_SUCCESS)
				{
					//throw std::logic_error("rt vkCreateRayTracingPipelines failed");
				}

				std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

				m_ShaderBindingTable.compile(queue, cmdBuff, *m_pipeline);

			}

			void RayTracingPipeline::trace(Framework::CommandBuffer& cmdbuff) {
				vkCmdBindPipeline(cmdbuff.getHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *m_pipeline);
				vkCmdBindDescriptorSets(cmdbuff.getHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *m_pipelineLayout, 0, (uint32_t)m_descriptorSets.size(), m_descriptorSets.data(), 0, 0);

				VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};
                
				vkCmdTraceRaysKHR(
					cmdbuff.getHandle(),
					&m_ShaderBindingTable.raygenShaderBindingTable(),
					&m_ShaderBindingTable.missShaderBindingTable(),
					&m_ShaderBindingTable.hitShaderBindingTable(),
					&callableShaderSbtEntry,
					m_width,
					m_height,
					1);
			}




			void RayTracingPipeline::setMaxRecursion(uint16_t recursion) {
				m_maxRecursion = recursion;
			}

			void RayTracingPipeline::addAccelerationStructure(TopLevelAS* AS, VkShaderStageFlags stage, uint32_t	binding) {
				m_AS.push_back({ AS, binding, stage });
			};



			void RayTracingPipeline::generateDescriptorLayout() {

				Framework::Device* d = Framework::Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				m_descriptorSetLayoutBinding = {};

				for (uint32_t i = 0; i < m_uniforms.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_uniforms[i].binding),
						VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						1,
						m_uniforms[i].stage,
						nullptr
						});
				}
				for (uint32_t i = 0; i < m_textures.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_textures[i].binding),
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						1,
						m_textures[i].stage,
						nullptr
						});
				}
				for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_frameBuffers[i].binding),
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						1,
						m_frameBuffers[i].stage,
						nullptr
						});
				}
				for (uint32_t i = 0; i < m_attachments.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_attachments[i].binding),
						VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
						1,
						m_attachments[i].stage,
						nullptr
						});
				}
				for (uint32_t i = 0; i < m_storageImages.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_storageImages[i].binding),
						VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
						1,
						m_storageImages[i].stage,
						nullptr
						});
				}
				for (uint32_t i = 0; i < m_texelBuffers.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_texelBuffers[i].binding),
						VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
						1,
						m_texelBuffers[i].stage,
						nullptr
						});
				}
				for (uint32_t i = 0; i < m_buffers.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_buffers[i].binding),
						VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
						1,
						m_buffers[i].stage,
						nullptr
						});
				}
				for (uint32_t i = 0; i < m_AS.size(); i++) {
					m_descriptorSetLayoutBinding.push_back({
						uint32_t(m_AS[i].binding),
						VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
						1,
						m_AS[i].stage,
						nullptr
						});
				}

				InitVkDestroyer(logical, m_descriptorSetLayout);
				if (!LavaCake::Core::CreateDescriptorSetLayout(logical, m_descriptorSetLayoutBinding, *m_descriptorSetLayout)) {
					Framework::ErrorCheck::setError((char*)"Can't create descriptor set layout");
				}

				m_descriptorCount = static_cast<uint32_t>(m_uniforms.size() + m_textures.size() + m_storageImages.size() + m_attachments.size() + m_frameBuffers.size() + m_texelBuffers.size() + m_buffers.size() + m_AS.size());
				if (m_descriptorCount == 0) return;

				m_descriptorPoolSize = {};

				if (m_uniforms.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
							uint32_t(m_uniforms.size())
						});
				}
				if (m_textures.size() + m_frameBuffers.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							uint32_t(m_textures.size() + m_frameBuffers.size())
						});
				}
				if (m_attachments.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
							uint32_t(m_attachments.size())
						});
				}
				if (m_storageImages.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
							uint32_t(m_storageImages.size())
						});
				}
				if (m_texelBuffers.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
							uint32_t(m_texelBuffers.size())
						});
				}
				if (m_buffers.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
							uint32_t(m_buffers.size())
						});
				}
				if (m_AS.size() > 0) {
					m_descriptorPoolSize.push_back({
						VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
							uint32_t(m_AS.size())
						});
				}
				InitVkDestroyer(logical, m_descriptorPool);
				if (!LavaCake::Core::CreateDescriptorPool(logical, false, m_descriptorCount, m_descriptorPoolSize, *m_descriptorPool)) {
					Framework::ErrorCheck::setError((char*)"Can't create descriptor pool");
				}

				if (!LavaCake::Core::AllocateDescriptorSets(logical, *m_descriptorPool, { *m_descriptorSetLayout }, m_descriptorSets)) {
					Framework::ErrorCheck::setError((char*)"Can't allocate descriptor set");
				}
				m_bufferDescriptorUpdate = { };
				int descriptorCount = 0;
				for (uint32_t i = 0; i < m_uniforms.size(); i++) {
					m_bufferDescriptorUpdate.push_back({
						m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
						uint32_t(m_uniforms[i].binding),								// uint32_t                             TargetDescriptorBinding
						0,																							// uint32_t                             TargetArrayElement
						VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,							// VkDescriptorType                     TargetDescriptorType
						{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
							{
								m_uniforms[i].buffer->getHandle(),					// VkBuffer                             buffer
								0,																					// VkDeviceSize                         offset
								VK_WHOLE_SIZE																// VkDeviceSize                         range
							}
						}
						});
				}

				m_imageDescriptorUpdate = { };
				for (uint32_t i = 0; i < m_textures.size(); i++) {
					m_imageDescriptorUpdate.push_back({
						m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
						uint32_t(m_textures[i].binding),								// uint32_t                             TargetDescriptorBinding
						0,																							// uint32_t                             TargetArrayElement
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,			// VkDescriptorType                     TargetDescriptorType
						{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
							{
								m_textures[i].t->getSampler(),							// vkSampler                            buffer
								m_textures[i].t->getImageView(),						// VkImageView                          offset
								m_textures[i].t->getLayout()								// VkImageLayout                         range
							}
						}
						});
				}
				for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
					std::vector<VkDescriptorImageInfo> descriptorInfo;
					descriptorInfo.push_back(
						{
							m_frameBuffers[i].f->getSampler(),
							m_frameBuffers[i].f->getImageViews(m_frameBuffers[i].viewIndex),
							m_frameBuffers[i].f->getLayout(m_frameBuffers[i].viewIndex),
						});

					m_imageDescriptorUpdate.push_back({
						m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
						uint32_t(m_frameBuffers[i].binding),						// uint32_t                             TargetDescriptorBinding
						0,																							// uint32_t                             TargetArrayElement
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,			// VkDescriptorType                     TargetDescriptorType
						descriptorInfo																	// std::vector<VkDescriptorBufferInfo>  BufferInfos
						});
				}
				for (uint32_t i = 0; i < m_attachments.size(); i++) {
					m_imageDescriptorUpdate.push_back({
							m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
							uint32_t(m_attachments[i].binding),								// uint32_t                             TargetDescriptorBinding
							0,																							// uint32_t                             TargetArrayElement
							VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,						// VkDescriptorType                     TargetDescriptorType
							{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
								{
									VK_NULL_HANDLE,															// vkSampler                            buffer
									m_attachments[i].a->getImageView(),					// VkImageView                          offset
									m_attachments[i].a->getLayout()							// VkImageLayout                         range
								}
							}
						});
				}

				for (uint32_t i = 0; i < m_storageImages.size(); i++) {
					m_imageDescriptorUpdate.push_back({
							m_descriptorSets[descriptorCount],							// VkDescriptorSet                      TargetDescriptorSet
							uint32_t(m_storageImages[i].binding),						// uint32_t                             TargetDescriptorBinding
							0,																							// uint32_t                             TargetArrayElement
							VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,								// VkDescriptorType                     TargetDescriptorType
							{																								// std::vector<VkDescriptorBufferInfo>  BufferInfos
								{
									VK_NULL_HANDLE,															// vkSampler                            buffer
									m_storageImages[i].s->getImageView(),				// VkImageView                          offset
									m_storageImages[i].s->getLayout()						// VkImageLayout                         range
								}
							}
						});
				}

				m_texelBufferDescriptorUpdate = {};
				for (uint32_t i = 0; i < m_texelBuffers.size(); i++) {

					LavaCake::Core::TexelBufferDescriptorInfo info = {
						m_descriptorSets[descriptorCount],
						uint32_t(m_texelBuffers[i].binding),
						0,
						VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
						{
							m_texelBuffers[i].t->getBufferView()
						}
					};

					m_texelBufferDescriptorUpdate.push_back(info);
				}


				for (uint32_t i = 0; i < m_buffers.size(); i++) {

					LavaCake::Core::BufferDescriptorInfo info = {
						m_descriptorSets[descriptorCount],
						uint32_t(m_buffers[i].binding),
						0,
						VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
						{
							{
								m_buffers[i].t->getHandle(),
								0,
								VK_WHOLE_SIZE
							}
						}
					};

					m_bufferDescriptorUpdate.push_back(info);
				}

				/////////////////////////////////////////////////////////////////////////////////////////////
				std::vector<VkWriteDescriptorSet> write_descriptors;
				std::vector<VkCopyDescriptorSet> copy_descriptors;

				// image descriptors
				for (auto& image_descriptor : m_imageDescriptorUpdate) {
					write_descriptors.push_back({
						VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
						nullptr,                                                                // const void                     * pNext
						image_descriptor.TargetDescriptorSet,                                   // VkDescriptorSet                  dstSet
						image_descriptor.TargetDescriptorBinding,                               // uint32_t                         dstBinding
						image_descriptor.TargetArrayElement,                                    // uint32_t                         dstArrayElement
						static_cast<uint32_t>(image_descriptor.ImageInfos.size()),              // uint32_t                         descriptorCount
						image_descriptor.TargetDescriptorType,                                  // VkDescriptorType                 descriptorType
						image_descriptor.ImageInfos.data(),                                     // const VkDescriptorImageInfo    * pImageInfo
						nullptr,                                                                // const VkDescriptorBufferInfo   * pBufferInfo
						nullptr                                                                 // const VkBufferView             * pTexelBufferView
						});
				}

				// buffer descriptors
				for (auto& buffer_descriptor : m_bufferDescriptorUpdate) {
					write_descriptors.push_back({
						VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
						nullptr,                                                                // const void                     * pNext
						buffer_descriptor.TargetDescriptorSet,                                  // VkDescriptorSet                  dstSet
						buffer_descriptor.TargetDescriptorBinding,                              // uint32_t                         dstBinding
						buffer_descriptor.TargetArrayElement,                                   // uint32_t                         dstArrayElement
						static_cast<uint32_t>(buffer_descriptor.BufferInfos.size()),            // uint32_t                         descriptorCount
						buffer_descriptor.TargetDescriptorType,                                 // VkDescriptorType                 descriptorType
						nullptr,                                                                // const VkDescriptorImageInfo    * pImageInfo
						buffer_descriptor.BufferInfos.data(),                                   // const VkDescriptorBufferInfo   * pBufferInfo
						nullptr                                                                 // const VkBufferView             * pTexelBufferView
						});
				}

				// texel buffer descriptors
				for (auto& texel_buffer_descriptor : m_texelBufferDescriptorUpdate) {
					write_descriptors.push_back({
						VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
						nullptr,                                                                // const void                     * pNext
						texel_buffer_descriptor.TargetDescriptorSet,                            // VkDescriptorSet                  dstSet
						texel_buffer_descriptor.TargetDescriptorBinding,                        // uint32_t                         dstBinding
						texel_buffer_descriptor.TargetArrayElement,                             // uint32_t                         dstArrayElement
						static_cast<uint32_t>(texel_buffer_descriptor.TexelBufferViews.size()), // uint32_t                         descriptorCount
						texel_buffer_descriptor.TargetDescriptorType,                           // VkDescriptorType                 descriptorType
						nullptr,                                                                // const VkDescriptorImageInfo    * pImageInfo
						nullptr,                                                                // const VkDescriptorBufferInfo   * pBufferInfo
						texel_buffer_descriptor.TexelBufferViews.data()                         // const VkBufferView             * pTexelBufferView
						});
				}


				std::vector<VkWriteDescriptorSetAccelerationStructureKHR> descriptorAccelerationStructureInfos{};

				for (auto& AS_descriptor : m_AS) {
					descriptorAccelerationStructureInfos.push_back({
						VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
						nullptr,
						1,
						&AS_descriptor.AS->getHandle()
						});
				}


				// acceleration structure descriptors
				for (int i = 0; i < m_AS.size(); i++) {
					write_descriptors.push_back({
						VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
						&descriptorAccelerationStructureInfos[i],																																// const void                     * pNext
						m_descriptorSets[descriptorCount],																			// VkDescriptorSet                  dstSet
						m_AS[i].binding,																												// uint32_t                         dstBinding
						0,																																			// uint32_t                         dstArrayElement
						static_cast<uint32_t>(1),																								// uint32_t                         descriptorCount
						VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,                          // VkDescriptorType                 descriptorType
						nullptr,                                                                // const VkDescriptorImageInfo    * pImageInfo
						nullptr,                                                                // const VkDescriptorBufferInfo   * pBufferInfo
						{	}
						});
				}

				vkUpdateDescriptorSets(logical, static_cast<uint32_t>(write_descriptors.size()), write_descriptors.data(), static_cast<uint32_t>(copy_descriptors.size()), copy_descriptors.data());
			}
	}
}
#endif
