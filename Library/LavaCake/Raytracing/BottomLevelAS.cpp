#include <LavaCake/Raytracing/BottomLevelAS.h>

namespace LavaCake {
	namespace RayTracing {

			void BottomLevelAccelerationStructure::addVertexBuffer(
				std::shared_ptr<Framework::VertexBuffer> vertexBuffer, 
				std::shared_ptr<Framework::Buffer> transformBuffer, bool opaque ) {

				VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

				vertexBufferDeviceAddress.deviceAddress = vertexBuffer->getVertexBuffer()->getBufferDeviceAddress();
				if (vertexBuffer->isIndexed()) {
					indexBufferDeviceAddress.deviceAddress = vertexBuffer->getIndexBuffer()->getBufferDeviceAddress();
				}
				transformBufferDeviceAddress.deviceAddress = transformBuffer->getBufferDeviceAddress();

				VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
				accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				accelerationStructureGeometry.pNext = nullptr;
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
					m_primPerGeometry.emplace_back( (uint32_t)vertexBuffer->getIndicesNumber() / 3);
					
				}
				else {
					m_primPerGeometry.emplace_back( (uint32_t)vertexBuffer->getVerticiesNumber() / 3);
				}
				m_primCount+=m_primPerGeometry[m_primPerGeometry.size()-1];
				m_geometry.push_back(accelerationStructureGeometry);
			}

			void BottomLevelAccelerationStructure::addVertexBuffer(
				const Framework::VertexBuffer& vertexBuffer, 
				const Framework::Buffer& transformBuffer , bool opaque ) {

				VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
				VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

				vertexBufferDeviceAddress.deviceAddress = vertexBuffer.getVertexBuffer()->getBufferDeviceAddress();
				if (vertexBuffer.isIndexed()) {
					indexBufferDeviceAddress.deviceAddress = vertexBuffer.getIndexBuffer()->getBufferDeviceAddress();
				}
				transformBufferDeviceAddress.deviceAddress = transformBuffer.getBufferDeviceAddress();

				VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
				accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				accelerationStructureGeometry.pNext = nullptr;
				accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
				accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
				accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
				accelerationStructureGeometry.geometry.triangles.maxVertex = (uint32_t)vertexBuffer.getVerticiesNumber();
				accelerationStructureGeometry.geometry.triangles.vertexStride = vertexBuffer.getByteStrideSize();
				if (vertexBuffer.isIndexed()) {
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
					accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
				}
				else {
					accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
				}
				accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
				accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
				accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

				if (vertexBuffer.isIndexed()) {
					m_primPerGeometry.emplace_back( (uint32_t)vertexBuffer.getIndicesNumber() / 3);
					
				}
				else {
					m_primPerGeometry.emplace_back( (uint32_t)vertexBuffer.getVerticiesNumber() / 3);
				}
				m_primCount+=m_primPerGeometry[m_primPerGeometry.size()-1];
				m_geometry.push_back(accelerationStructureGeometry);
			}


			void BottomLevelAccelerationStructure::addAabbBuffer(uint64_t aabbBufferDeviceAdressse,int primitiveCount, bool opaque){
				

				VkAccelerationStructureGeometryKHR geometry = {};
					geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
					geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
					geometry.geometry.aabbs = {
						VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
						nullptr,
						aabbBufferDeviceAdressse,
						sizeof(VkAabbPositionsKHR),
					};
					if(opaque){
						geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
					}
				m_primCount+=primitiveCount;
				m_primPerGeometry.emplace_back(primitiveCount);
				m_geometry.push_back(geometry);
			}

			void BottomLevelAccelerationStructure::allocate(const  Framework::Queue& queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate) {

				Framework::Device* d = Framework::Device::getDevice();
				VkDevice device = d->getLogicalDevice();
				VkPhysicalDevice phyDevice = d->getPhysicalDevice();

				VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
				accelerationStructureBuildGeometryInfo.pNext =nullptr;
				accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

				if(allowUpdate)
					accelerationStructureBuildGeometryInfo.flags = accelerationStructureBuildGeometryInfo.flags | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;

				accelerationStructureBuildGeometryInfo.geometryCount = (uint32_t)m_geometry.size();
				accelerationStructureBuildGeometryInfo.pGeometries = m_geometry.data();


				VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
				accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
				vkGetAccelerationStructureBuildSizesKHR(
					device,
					VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
					&accelerationStructureBuildGeometryInfo,
					m_primPerGeometry.data(),
					&accelerationStructureBuildSizesInfo);

				m_ASBuffer = std::make_shared<Framework::Buffer>(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);


				VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
				accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
				accelerationStructureCreateInfo.buffer = m_ASBuffer->getHandle();
				accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
				accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				vkCreateAccelerationStructureKHR(device, &accelerationStructureCreateInfo, nullptr, &m_accelerationStructure);


				auto scratch_size = accelerationStructureBuildSizesInfo.buildScratchSize;
				if(allowUpdate){
					scratch_size = std::max( accelerationStructureBuildSizesInfo.updateScratchSize, accelerationStructureBuildSizesInfo.buildScratchSize);
				}
				m_scratchBuffer = std::make_shared<Framework::Buffer>(scratch_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

				VkBufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
				scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				scratchBufferDeviceAddressInfo.buffer = m_scratchBuffer->getHandle();

				VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
				accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
				if(allowUpdate)
					accelerationBuildGeometryInfo.flags = accelerationBuildGeometryInfo.flags | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;

				accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		
				accelerationBuildGeometryInfo.dstAccelerationStructure = m_accelerationStructure;
				accelerationBuildGeometryInfo.geometryCount = (uint32_t)m_geometry.size();
				accelerationBuildGeometryInfo.pGeometries = m_geometry.data();
				accelerationBuildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(device, &scratchBufferDeviceAddressInfo);


				std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfosRef;
				std::vector<VkAccelerationStructureBuildRangeInfoKHR> accelerationBuildStructureRangeInfos;
				for(int i = 0; i < m_geometry.size(); i ++){
					VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
					accelerationStructureBuildRangeInfo.primitiveCount = m_primCount;
					accelerationStructureBuildRangeInfo.primitiveOffset = 0;
					accelerationStructureBuildRangeInfo.firstVertex = 0;
					accelerationStructureBuildRangeInfo.transformOffset = 0;

					accelerationBuildStructureRangeInfos.push_back(accelerationStructureBuildRangeInfo);
					accelerationBuildStructureRangeInfosRef.push_back(&accelerationBuildStructureRangeInfos[i]);
				}
				

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
						accelerationBuildStructureRangeInfosRef.data());
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
						accelerationBuildStructureRangeInfosRef.data());
					cmdBuff.endRecord();

					cmdBuff.submit(queue, {}, {});

					cmdBuff.wait(UINT64_MAX);
					cmdBuff.resetFence();
				}

				m_accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
				m_accelerationDeviceAddressInfo.accelerationStructure = m_accelerationStructure;
				m_accelerationDeviceAddressInfo.pNext = nullptr;
				m_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &m_accelerationDeviceAddressInfo);

			}


			void BottomLevelAccelerationStructure::update(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff){

				Framework::Device* d = Framework::Device::getDevice();
				VkDevice device = d->getLogicalDevice();
				VkPhysicalDevice phyDevice = d->getPhysicalDevice();

				VkBufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
				scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				scratchBufferDeviceAddressInfo.buffer = m_scratchBuffer->getHandle();

				VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
				accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
				accelerationBuildGeometryInfo.mode =  VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
				accelerationBuildGeometryInfo.srcAccelerationStructure = m_accelerationStructure;
				accelerationBuildGeometryInfo.dstAccelerationStructure = m_accelerationStructure;
				accelerationBuildGeometryInfo.geometryCount = (uint32_t)m_geometry.size();
				accelerationBuildGeometryInfo.pGeometries = m_geometry.data();
				accelerationBuildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(device, &scratchBufferDeviceAddressInfo);


				std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfosRef;
				std::vector<VkAccelerationStructureBuildRangeInfoKHR> accelerationBuildStructureRangeInfos;
				for(int i = 0; i < m_geometry.size(); i ++){
					VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
					accelerationStructureBuildRangeInfo.primitiveCount = m_primCount;
					accelerationStructureBuildRangeInfo.primitiveOffset = 0;
					accelerationStructureBuildRangeInfo.firstVertex = 0;
					accelerationStructureBuildRangeInfo.transformOffset = 0;

					accelerationBuildStructureRangeInfos.push_back(accelerationStructureBuildRangeInfo);
					accelerationBuildStructureRangeInfosRef.push_back(&accelerationBuildStructureRangeInfos[i]);
				}
				


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
						accelerationBuildStructureRangeInfosRef.data());
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
						accelerationBuildStructureRangeInfosRef.data());
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

			const VkAccelerationStructureKHR& BottomLevelAccelerationStructure::getHandle() const {
				return m_accelerationStructure;
			}

			uint64_t BottomLevelAccelerationStructure::getDeviceAddress() const {
				return m_deviceAddress;
			}

			uint32_t BottomLevelAccelerationStructure::getPrimitiveNumber() const {
				return (uint32_t)m_geometry.size();
			}



		void updateBLAS(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff, std::vector<BottomLevelAccelerationStructure>& blas){

			Framework::Device* d = Framework::Device::getDevice();
			VkDevice device = d->getLogicalDevice();
			VkPhysicalDevice phyDevice = d->getPhysicalDevice();

			std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos(blas.size());
			std::vector<VkBufferDeviceAddressInfoKHR> scratchBufferDeviceAddressInfo(blas.size());
			std::vector<VkAccelerationStructureBuildGeometryInfoKHR> accelerationBuildGeometryInfo(blas.size());
			int ascount = 0;
			for ( int i = 0; i < blas.size(); i++){
				scratchBufferDeviceAddressInfo[i].sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				scratchBufferDeviceAddressInfo[i].buffer =  blas[i].getScratchBuffer()->getHandle();

				
				accelerationBuildGeometryInfo[i].sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				accelerationBuildGeometryInfo[i].type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				accelerationBuildGeometryInfo[i].flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
				accelerationBuildGeometryInfo[i].mode =  VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
				accelerationBuildGeometryInfo[i].srcAccelerationStructure = blas[i].getHandle();
				accelerationBuildGeometryInfo[i].dstAccelerationStructure = blas[i].getHandle();
				accelerationBuildGeometryInfo[i].geometryCount = (uint32_t)blas[i].getGeometry().size();
				accelerationBuildGeometryInfo[i].pGeometries = blas[i].getGeometry().data();
				accelerationBuildGeometryInfo[i].scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(device, &scratchBufferDeviceAddressInfo[i]);

				for(int j = 0; j < blas[i].getGeometry().size(); j ++){
					accelerationBuildStructureRangeInfos[ascount] = new VkAccelerationStructureBuildRangeInfoKHR();
					accelerationBuildStructureRangeInfos[ascount]->primitiveCount = blas[i].getPrimCount();
					accelerationBuildStructureRangeInfos[ascount]->primitiveOffset = 0;
					accelerationBuildStructureRangeInfos[ascount]->firstVertex = 0;
					accelerationBuildStructureRangeInfos[ascount]->transformOffset = 0;
					ascount++;
				}
			
			}

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
					accelerationBuildGeometryInfo.size(),
					accelerationBuildGeometryInfo.data(),
					accelerationBuildStructureRangeInfos.data());
			}
			else
			{
				// Acceleration structure needs to be build on the device
				cmdBuff.resetFence();
				cmdBuff.beginRecord();
				vkCmdBuildAccelerationStructuresKHR(
					cmdBuff.getHandle(),
					accelerationBuildGeometryInfo.size(),
					accelerationBuildGeometryInfo.data(),
					accelerationBuildStructureRangeInfos.data());
				cmdBuff.endRecord();

				cmdBuff.submit(queue, {}, {});
				
				cmdBuff.wait(UINT64_MAX);	
				cmdBuff.resetFence();
			}
			for ( int i = 0; i < blas.size(); i++){
				delete accelerationBuildStructureRangeInfos[i];
			}
			//m_accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			//m_accelerationDeviceAddressInfo.accelerationStructure = m_accelerationStructure;
			//m_accelerationDeviceAddressInfo.pNext = nullptr;
			//m_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &m_accelerationDeviceAddressInfo);
		}

	}
}