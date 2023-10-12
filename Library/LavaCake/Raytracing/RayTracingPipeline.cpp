#include <LavaCake/Raytracing/RayTracingPipeline.h>

namespace LavaCake {
	namespace RayTracing {



			void RayTracingPipeline::addRayGenModule(const RayGenShaderModule& module) {
				if (m_isHitGroupOpen)
				{
					Framework::ErrorCheck::setError("Cannot add raygen stage in when hit group open");
					return;
				}

				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
				stageCreate.module = module.getStageParameter().shaderModule;
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

			void RayTracingPipeline::addMissModule(const MissShaderModule& module) {
				if (m_isHitGroupOpen)
				{
					Framework::ErrorCheck::setError("Cannot add miss stage in when hit group open");
					return;
				}

				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
				stageCreate.module = module.getStageParameter().shaderModule;
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

			void RayTracingPipeline::setClosestHitModule(const ClosestHitShaderModule& module) {
				if (!m_isHitGroupOpen) {
					Framework::ErrorCheck::setError("No open hitgroup");
					return;
				}
				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage =  module.getStageParameter().shaderStage;
				stageCreate.module = module.getStageParameter().shaderModule;
				stageCreate.pName =  module.getStageParameter().entryPointName;
				stageCreate.flags = 0;
				stageCreate.pSpecializationInfo = nullptr;

				m_shaderStages.emplace_back(stageCreate);
				uint32_t shaderIndex = static_cast<uint32_t>(m_shaderStages.size() - 1);
				m_shaderGroups[m_shaderGroups.size() - 1].closestHitShader = shaderIndex;
			}

			void RayTracingPipeline::setAnyHitModule(const AnyHitShaderModule& module) {
				if (!m_isHitGroupOpen) {
					Framework::ErrorCheck::setError("No open hitgroup");
					return;
				}
				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage =  module.getStageParameter().shaderStage;
				stageCreate.module = module.getStageParameter().shaderModule;
				stageCreate.pName =  module.getStageParameter().entryPointName;
				stageCreate.flags = 0;
				stageCreate.pSpecializationInfo = nullptr;

				m_shaderStages.emplace_back(stageCreate);
				uint32_t shaderIndex = static_cast<uint32_t>(m_shaderStages.size() - 1);
				m_shaderGroups[m_shaderGroups.size() - 1].anyHitShader = shaderIndex;
			}

			void RayTracingPipeline::setIntersectionModule(const IntersectionShaderModule& module) {
				if (m_isHitGroupOpen) {
					Framework::ErrorCheck::setError("No open hitgroup");
					return;
				}
				VkPipelineShaderStageCreateInfo stageCreate;
				stageCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageCreate.pNext = nullptr;
				stageCreate.stage =  module.getStageParameter().shaderStage;
				stageCreate.module = module.getStageParameter().shaderModule;
				stageCreate.pName =  module.getStageParameter().entryPointName;
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

			void RayTracingPipeline::compile(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff) {

				Framework::Device* d = Framework::Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

				std::vector<VkDescriptorSetLayout> layouts= { m_descriptorSetLayout };
				
				if (!CreatePipelineLayout(logical, layouts, {}, m_pipelineLayout)) {
						Framework::ErrorCheck::setError("Can't create compute pipeline layout");
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

			void RayTracingPipeline::trace(Framework::CommandBuffer& cmdbuff, uint32_t dimX, uint32_t dimY, uint32_t dimZ) {
				vkCmdBindPipeline(cmdbuff.getHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline);

				
				VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};
                
				vkCmdTraceRaysKHR(
					cmdbuff.getHandle(),
					&m_ShaderBindingTable.raygenShaderBindingTable(),
					&m_ShaderBindingTable.missShaderBindingTable(),
					&m_ShaderBindingTable.hitShaderBindingTable(),
					&callableShaderSbtEntry,
					dimX,
					dimY,
					dimZ);
			}

			void RayTracingPipeline::bindPipeline( Framework::CommandBuffer& cmdBuff){
				vkCmdBindPipeline(cmdBuff.getHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_pipeline);
			}

			void RayTracingPipeline::bindDescriptorSet( Framework::CommandBuffer& cmdBuffer, const  Framework::DescriptorSet& descriptorSet){
				std::vector<VkDescriptorSet> descriptorSets = { descriptorSet.getHandle() };
				vkCmdBindDescriptorSets(cmdBuffer.getHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_pipelineLayout, 0,
							static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
							0, {});
			}

			void RayTracingPipeline::setMaxRecursion(uint16_t recursion) {
				m_maxRecursion = recursion;
			}


	}
}
