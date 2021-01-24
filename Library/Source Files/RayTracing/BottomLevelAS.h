#pragma once
#include "Framework/VertexBuffer.h"

namespace LavaCake {
	namespace RayTracing {

		uint64_t getBufferDeviceAddress(VkBuffer buffer)
		{
			Framework::Device* d = Framework::Device::getDevice();
			VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
			bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufferDeviceAI.buffer = buffer;
			return vkGetBufferDeviceAddressKHR(d->getLogicalDevice(), &bufferDeviceAI);
		}

		class BottomLevelAS {
		public :
			BottomLevelAS() {

			};


			void addVertexBuffer(Framework::VertexBuffer* vertexBuffer, Framework::TransformBuffer* transformBuffer, bool opaque = true) {

				VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

				vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer->getVertexBuffer().getHandle());
				if (vertexBuffer->isIndexed()) {
					indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer->getIndexBuffer().getHandle());
				}
				transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer->getBuffer().getHandle());

				VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
				accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
				accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
				accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
				accelerationStructureGeometry.geometry.triangles.maxVertex = vertexBuffer->getVerticiesNumber();
				accelerationStructureGeometry.geometry.triangles.vertexStride = vertexBuffer->getByteStrideSize();
				if (vertexBuffer->isIndexed()) {
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
					accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
				}
				accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
				accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
				accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;


				m_geometry.push_back(accelerationStructureGeometry);
			}


			void allocate(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, mat4 transform, bool allowUpdate = false) {

				Framework::Device* d = Framework::Device::getDevice();
				VkDevice device = d->getLogicalDevice();
				VkPhysicalDevice phyDevice= d->getPhysicalDevice();

				VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
				accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
				accelerationStructureBuildGeometryInfo.geometryCount = m_geometry.size();
				accelerationStructureBuildGeometryInfo.pGeometries = m_geometry.data();

				uint32_t numTriangles = 0;

				for (size_t i = 0; i < m_geometry.size(); i++) {
					numTriangles += m_geometry[i].geometry.triangles.maxVertex;
				}

				VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
				accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
				vkGetAccelerationStructureBuildSizesKHR(
					device,
					VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
					&accelerationStructureBuildGeometryInfo,
					&numTriangles,
					&accelerationStructureBuildSizesInfo);

				m_ASBuffer.allocate(queue, cmdBuff, accelerationStructureBuildSizesInfo.accelerationStructureSize, VkBufferUsageFlagBits(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));


				VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
				accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
				accelerationStructureCreateInfo.buffer = m_ASBuffer.getHandle();
				accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
				accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				vkCreateAccelerationStructureKHR(device, &accelerationStructureCreateInfo, nullptr, &m_accelerationStructure);


				Framework::Buffer scratchBuffer;
				scratchBuffer.allocate(queue, cmdBuff, accelerationStructureBuildSizesInfo.buildScratchSize, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));

				VkBufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
				scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				scratchBufferDeviceAddressInfo.buffer = scratchBuffer.getHandle();


				VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
				accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
				accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
				accelerationBuildGeometryInfo.dstAccelerationStructure = m_accelerationStructure;
				accelerationBuildGeometryInfo.geometryCount = m_geometry.size();
				accelerationBuildGeometryInfo.pGeometries = m_geometry.data();
				accelerationBuildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(device, &scratchBufferDeviceAddressInfo);

				VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
				accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
				accelerationStructureBuildRangeInfo.primitiveOffset = 0;
				accelerationStructureBuildRangeInfo.firstVertex = 0;
				accelerationStructureBuildRangeInfo.transformOffset = 0;
				std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

				VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
				accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
				VkPhysicalDeviceFeatures2 deviceFeatures2{};
				deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				deviceFeatures2.pNext = &accelerationStructureFeatures;
				vkGetPhysicalDeviceFeatures2(phyDevice, &deviceFeatures2);

				if (accelerationStructureFeatures.accelerationStructureHostCommands)
				{
					// Implementation supports building acceleration structure building on host
					vkBuildAccelerationStructuresKHR(
						device,
						VK_NULL_HANDLE,
						1,
						&accelerationBuildGeometryInfo,
						accelerationBuildStructureRangeInfos.data());
				}
				else
				{
					// Acceleration structure needs to be build on the device
					cmdBuff.resetFence();
					cmdBuff.beginRecord();
					vkCmdBuildAccelerationStructuresKHR(
						cmdBuff.getHandle(),
						1,
						&accelerationBuildGeometryInfo,
						accelerationBuildStructureRangeInfos.data());
					cmdBuff.endRecord();


					if (!LavaCake::Core::SubmitCommandBuffersToQueue(queue->getHandle(), {}, { cmdBuff.getHandle() }, {}, cmdBuff.getFence())) {
						//todo : use errocheck
					}

					cmdBuff.wait(MAXINT32);
					cmdBuff.resetFence();
				}

				m_accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
				m_accelerationDeviceAddressInfo.accelerationStructure = m_accelerationStructure;
				m_accelerationDeviceAddressInfo.pNext = nullptr;
				m_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &m_accelerationDeviceAddressInfo);

			}


			VkAccelerationStructureKHR& getHandle() {
				return m_accelerationStructure;
			}

			uint64_t getDeviceAddress() {
				return m_deviceAddress;
			}

			uint32_t getPrimitiveNumber() {
				return (uint32_t)m_geometry.size();
			}

		private:

			

			Framework::Buffer m_ASBuffer;
			VkAccelerationStructureKHR m_accelerationStructure{};
			uint64_t m_deviceAddress;
			VkAccelerationStructureDeviceAddressInfoKHR m_accelerationDeviceAddressInfo{};
			std::vector<VkAccelerationStructureGeometryKHR> m_geometry;
		};

	}
}