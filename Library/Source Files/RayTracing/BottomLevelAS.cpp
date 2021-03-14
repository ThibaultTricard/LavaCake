#ifdef RAYTRACING
#include "BottomLevelAS.h"

namespace LavaCake {
	namespace RayTracing {

			void BottomLevelAS::addVertexBuffer(Framework::VertexBuffer* vertexBuffer, Framework::TransformBuffer* transformBuffer, bool opaque ) {

				VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

				vertexBufferDeviceAddress.deviceAddress = vertexBuffer->getVertexBuffer().getBufferDeviceAddress();
				if (vertexBuffer->isIndexed()) {
					indexBufferDeviceAddress.deviceAddress = vertexBuffer->getIndexBuffer().getBufferDeviceAddress();
				}
				transformBufferDeviceAddress.deviceAddress = transformBuffer->getBuffer().getBufferDeviceAddress();

				VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
				accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
				accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
				accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
				accelerationStructureGeometry.geometry.triangles.maxVertex = (uint32_t)vertexBuffer->getVerticiesNumber();
				accelerationStructureGeometry.geometry.triangles.vertexStride = vertexBuffer->getByteStrideSize();
				if (vertexBuffer->isIndexed()) {
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
					accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
				}
				else {
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
				}
				accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
				accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
				accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

				if (vertexBuffer->isIndexed()) {
					m_primCount += (uint32_t)vertexBuffer->getIndicesNumber();
				}
				else {
					m_primCount += (uint32_t)vertexBuffer->getVerticiesNumber();
				}

				m_geometry.push_back(accelerationStructureGeometry);
			}


			void BottomLevelAS::allocate(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate) {

				Framework::Device* d = Framework::Device::getDevice();
				VkDevice device = d->getLogicalDevice();
				VkPhysicalDevice phyDevice = d->getPhysicalDevice();

				VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
				accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
				accelerationStructureBuildGeometryInfo.geometryCount = (uint32_t)m_geometry.size();
				accelerationStructureBuildGeometryInfo.pGeometries = m_geometry.data();


				uint32_t vertexNumber = 0;
				for (size_t i = 0; i < m_geometry.size(); i++) {
					vertexNumber += m_geometry[i].geometry.triangles.maxVertex;
				}



				VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
				accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
				vkGetAccelerationStructureBuildSizesKHR(
					device,
					VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
					&accelerationStructureBuildGeometryInfo,
					&m_primCount,
					&accelerationStructureBuildSizesInfo);

				m_ASBuffer.allocate(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);


				VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
				accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
				accelerationStructureCreateInfo.buffer = m_ASBuffer.getHandle();
				accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
				accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				vkCreateAccelerationStructureKHR(device, &accelerationStructureCreateInfo, nullptr, &m_accelerationStructure);


				
				m_scratchBuffer.allocate(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

				VkBufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
				scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				scratchBufferDeviceAddressInfo.buffer = m_scratchBuffer.getHandle();

				VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
				accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
				accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
				accelerationBuildGeometryInfo.dstAccelerationStructure = m_accelerationStructure;
				accelerationBuildGeometryInfo.geometryCount = (uint32_t)m_geometry.size();
				accelerationBuildGeometryInfo.pGeometries = m_geometry.data();
				accelerationBuildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(device, &scratchBufferDeviceAddressInfo);

				VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
				accelerationStructureBuildRangeInfo.primitiveCount = m_primCount;
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

					cmdBuff.submit(queue, {}, {});


					cmdBuff.wait(UINT32_MAX);
					cmdBuff.resetFence();
				}

				m_accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
				m_accelerationDeviceAddressInfo.accelerationStructure = m_accelerationStructure;
				m_accelerationDeviceAddressInfo.pNext = nullptr;
				m_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &m_accelerationDeviceAddressInfo);

			}


			VkAccelerationStructureKHR& BottomLevelAS::getHandle() {
				return m_accelerationStructure;
			}

			uint64_t BottomLevelAS::getDeviceAddress() {
				return m_deviceAddress;
			}

			uint32_t BottomLevelAS::getPrimitiveNumber() {
				return (uint32_t)m_geometry.size();
			}

	}
}
#endif