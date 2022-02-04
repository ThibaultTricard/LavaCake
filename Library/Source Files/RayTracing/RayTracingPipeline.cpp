#include "RayTracingPipeline.h"

namespace LavaCake {
	namespace RayTracing {



			void RayTracingPipeline::addRayGenModule(RayGenShaderModule* module) {
				if (m_isHitGroupOpen)
				{
					Framework::ErrorCheck::setError("Cannot add raygen stage in when hit group open");
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
					Framework::ErrorCheck::setError("Cannot add miss stage in when hit group open");
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
					Framework::ErrorCheck::setError("Hit group already open");
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
					Framework::ErrorCheck::setError("No open hitgroup");
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
					Framework::ErrorCheck::setError("No open hitgroup");
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
					Framework::ErrorCheck::setError("No open hitgroup");
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
					Framework::ErrorCheck::setError("No hit group open");
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

				std::vector<VkDescriptorSetLayout> layouts= { m_descriptorSet->getLayout() };
				if (!m_descriptorSet->isEmpty()) {
					if (!CreatePipelineLayout(logical, layouts, {}, m_pipelineLayout)) {
						Framework::ErrorCheck::setError("Can't create compute pipeline layout");
					}
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
				rayPipelineInfo.layout = m_pipelineLayout;
				rayPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
				rayPipelineInfo.basePipelineIndex = 0;

				std::vector<VkRayTracingPipelineCreateInfoKHR> pipelineInfos = { rayPipelineInfo };

				std::vector<VkPipeline> pipelines(pipelineInfos.size());
				VkResult code = vkCreateRayTracingPipelinesKHR(logical, nullptr, nullptr, (uint32_t)pipelineInfos.size(), pipelineInfos.data(), nullptr, pipelines.data());
				m_pipeline = pipelines[0];



				if (code != VK_SUCCESS)
				{
					//throw std::logic_error("rt vkCreateRayTracingPipelines failed");
				}

				std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

				m_ShaderBindingTable.compile(queue, cmdBuff, m_pipeline);

			}

			void RayTracingPipeline::trace(Framework::CommandBuffer& cmdbuff) {
				vkCmdBindPipeline(cmdbuff.getHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline);

				std::vector<VkDescriptorSet> descriptorSets = { m_descriptorSet->getHandle() };
				if (!m_descriptorSet->isEmpty()) {
					vkCmdBindDescriptorSets(cmdbuff.getHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineLayout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), 0, 0);
				}
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


	}
}
