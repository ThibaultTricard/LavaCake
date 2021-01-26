#pragma once
#include "AllHeaders.h"
#include "BottomLevelAS.h"



namespace LavaCake {
  namespace RayTracing {
    
    class TopLevelAS {
    public:
      void addInstance(BottomLevelAS* bottomLevelAS, VkTransformMatrixKHR& transform, uint32_t instanceID, uint32_t hitGroupIndex) {
        m_instances.push_back({ bottomLevelAS ,transform , instanceID , hitGroupIndex });

        VkAccelerationStructureInstanceKHR instance{};
        instance.transform = transform;
        instance.instanceCustomIndex = 0;
        instance.mask = 0xFF;
        instance.instanceShaderBindingTableRecordOffset = 0;
        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instance.accelerationStructureReference = bottomLevelAS->getDeviceAddress();
        m_AccelerationStructureInstances.push_back(instance);

      }

      void alloctate(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate = false) {
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkPhysicalDevice phyDevice = d->getPhysicalDevice();
        std::vector<Framework::Buffer> instancesBuffers;
        std::vector<VkAccelerationStructureGeometryKHR> accelerationStructureGeometry;

        for (int i = 0; i < m_AccelerationStructureInstances.size(); i++) {
          // Buffer for instance data
          std::vector<VkAccelerationStructureInstanceKHR> instance = { m_AccelerationStructureInstances[i] };
          Framework::Buffer instancesBuffer;
          instancesBuffer.allocate(queue,
            cmdBuff,
            instance,
            VkBufferUsageFlagBits(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR),
            VK_FORMAT_R32_SFLOAT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

          instancesBuffers.push_back(instancesBuffer);

          VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
          instanceDataDeviceAddress.deviceAddress = instancesBuffer.getBufferDeviceAddress();

          VkAccelerationStructureGeometryKHR asg{};
          asg.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
          asg.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
          asg.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
          asg.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
          asg.geometry.instances.arrayOfPointers = VK_FALSE;
          asg.geometry.instances.data = instanceDataDeviceAddress;

          accelerationStructureGeometry.push_back(asg);
         
          m_primitive_count.push_back(m_instances[i].bottomLevelAS->getPrimitiveNumber());
        }

        // Get size info
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.geometryCount = (uint32_t)accelerationStructureGeometry.size();
        accelerationStructureBuildGeometryInfo.pGeometries = accelerationStructureGeometry.data();

        uint32_t primitive_count = 1;

        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};

        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(
          logical,
          VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
          &accelerationStructureBuildGeometryInfo,
          &primitive_count,
          &accelerationStructureBuildSizesInfo);

        createAccelerationStructure(queue, cmdBuff, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, accelerationStructureBuildSizesInfo);

        // Create a small scratch buffer used during build of the top level acceleration structure
        Framework::Buffer scratchBuffer;
        scratchBuffer.allocate(queue, cmdBuff, accelerationStructureBuildSizesInfo.buildScratchSize, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));
        
        VkBufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
        scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        scratchBufferDeviceAddressInfo.buffer = scratchBuffer.getHandle();


        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo = {}; 
        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationBuildGeometryInfo.dstAccelerationStructure = m_accelerationStructure;
        accelerationBuildGeometryInfo.geometryCount = (uint32_t)accelerationStructureGeometry.size();
        accelerationBuildGeometryInfo.pGeometries = accelerationStructureGeometry.data();
        accelerationBuildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(logical, &scratchBufferDeviceAddressInfo);

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = 1;
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
            logical,
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

      
      }

 
      VkAccelerationStructureKHR& getHandle() {
        return m_accelerationStructure;
      }

      void createAccelerationStructure(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
      {
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        m_ASBuffer = new Framework::Buffer();
        m_ASBuffer->allocate(queue, cmdBuff, buildSizeInfo.accelerationStructureSize, VkBufferUsageFlagBits(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));
        // Buffer and memory
       
        
        // Acceleration structure
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
        accelerationStructureCreate_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreate_info.buffer = m_ASBuffer->getHandle();
        accelerationStructureCreate_info.size = buildSizeInfo.accelerationStructureSize;
        accelerationStructureCreate_info.type = type;
        vkCreateAccelerationStructureKHR(logical, &accelerationStructureCreate_info, nullptr, &m_accelerationStructure);
        // AS device address
        VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
        accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        accelerationDeviceAddressInfo.accelerationStructure = m_accelerationStructure;
        m_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(logical, &accelerationDeviceAddressInfo);
      }


		private : 

    struct instance{
			BottomLevelAS* bottomLevelAS;
      VkTransformMatrixKHR transform;
      uint32_t instanceID;
      uint32_t hitGroupIndex;
    };


    /// Size of the buffer containing the instance descriptors
    VkDeviceSize m_instanceDescsSizeInBytes;
    /// Size of the buffer containing the TLAS
    VkDeviceSize m_resultSizeInBytes;

    std::vector<VkAccelerationStructureInstanceKHR> m_AccelerationStructureInstances;
		std::vector<instance> m_instances;

    uint64_t m_deviceAddress = 0;
    VkAccelerationStructureKHR m_accelerationStructure;

    std::vector<uint32_t> m_primitive_count;

    Framework::Buffer* m_ASBuffer;
		};
	}
}