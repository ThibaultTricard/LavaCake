#include <LavaCake/Raytracing/TopLevelAS.h>

namespace LavaCake {
  namespace RayTracing {

     
      void TopLevelAccelerationStructure::addInstance(BottomLevelAccelerationStructure* bottomLevelAS, VkTransformMatrixKHR& transform, uint32_t instanceID, uint32_t hitGroupOffset) {
        m_instances.push_back({ bottomLevelAS ,transform , instanceID , hitGroupOffset });

        VkAccelerationStructureInstanceKHR instance{};
        instance.transform = transform;
        instance.instanceCustomIndex = instanceID;
        instance.mask = 0xFF;
        instance.instanceShaderBindingTableRecordOffset = hitGroupOffset;
        instance.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR;
        instance.accelerationStructureReference = bottomLevelAS->getDeviceAddress();
        m_AccelerationStructureInstances.push_back(instance);

      }

      void TopLevelAccelerationStructure::alloctate(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate) {
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkPhysicalDevice phyDevice = d->getPhysicalDevice();
        
        m_instancesBuffer = Framework::Buffer(queue,
          cmdBuff,
          m_AccelerationStructureInstances,
          VkBufferUsageFlagBits(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR),
          VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        m_instanceDataDeviceAddress.deviceAddress = m_instancesBuffer.getBufferDeviceAddress();
        
        
        m_accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        m_accelerationStructureGeometry.pNext = nullptr;
        m_accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        m_accelerationStructureGeometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
        m_accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        m_accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
        m_accelerationStructureGeometry.geometry.instances.data = m_instanceDataDeviceAddress;

        // Get size info
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
        accelerationStructureBuildGeometryInfo.pNext = nullptr;
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        if(allowUpdate){
          accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        }
        else{
          accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR ;
        }
        accelerationStructureBuildGeometryInfo.geometryCount = 1;
        accelerationStructureBuildGeometryInfo.pGeometries = &m_accelerationStructureGeometry;

        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};

        uint32_t maxInstanceCount = (uint32_t)m_instances.size();
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(
          logical,
          VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
          &accelerationStructureBuildGeometryInfo,
          &maxInstanceCount,
          &accelerationStructureBuildSizesInfo);

        createAccelerationStructure( VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, accelerationStructureBuildSizesInfo);

        auto scratch_size = accelerationStructureBuildSizesInfo.buildScratchSize;
        if(allowUpdate){
          scratch_size = std::max( accelerationStructureBuildSizesInfo.updateScratchSize, accelerationStructureBuildSizesInfo.buildScratchSize);
        }
        // Create a small scratch buffer used during build of the top level acceleration structure
        m_scratchBuffer = std::make_shared<Framework::Buffer>(scratch_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        m_scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        m_scratchBufferDeviceAddressInfo.buffer = m_scratchBuffer->getHandle();


        m_accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        m_accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        m_accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
         if(allowUpdate){
          m_accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        }
        else{
          m_accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR ;
        }
        m_accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        m_accelerationBuildGeometryInfo.dstAccelerationStructure = m_accelerationStructure;
        m_accelerationBuildGeometryInfo.geometryCount = 1;
        m_accelerationBuildGeometryInfo.pGeometries = &m_accelerationStructureGeometry;
        m_accelerationBuildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(logical, &m_scratchBufferDeviceAddressInfo);

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = uint32_t(m_AccelerationStructureInstances.size());
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;
        m_accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };


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
            &m_accelerationBuildGeometryInfo,
            m_accelerationBuildStructureRangeInfos.data());
        }
        else
        {
          // Acceleration structure needs to be build on the device
          cmdBuff.resetFence();
          cmdBuff.beginRecord();
          vkCmdBuildAccelerationStructuresKHR(
            cmdBuff.getHandle(),
            1,
            &m_accelerationBuildGeometryInfo,
            m_accelerationBuildStructureRangeInfos.data());
          cmdBuff.endRecord();

          cmdBuff.submit(queue, {}, {});

          cmdBuff.wait(UINT32_MAX);
          cmdBuff.resetFence();
        }


      }

      void TopLevelAccelerationStructure::createAccelerationStructure(VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
      {
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        m_ASBuffer = std::make_shared<Framework::Buffer>(buildSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
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

      void TopLevelAccelerationStructure::update(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff){
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkPhysicalDevice phyDevice = d->getPhysicalDevice();

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = uint32_t(m_AccelerationStructureInstances.size());
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;
        m_accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

        m_accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        m_accelerationBuildGeometryInfo.srcAccelerationStructure = m_accelerationStructure;

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
            &m_accelerationBuildGeometryInfo,
            m_accelerationBuildStructureRangeInfos.data());
        }
        else
        {
          // Acceleration structure needs to be build on the device
          cmdBuff.resetFence();
          cmdBuff.beginRecord();
          vkCmdBuildAccelerationStructuresKHR(
            cmdBuff.getHandle(),
            1,
            &m_accelerationBuildGeometryInfo,
            m_accelerationBuildStructureRangeInfos.data());
          cmdBuff.endRecord();

          cmdBuff.submit(queue, {}, {});

          cmdBuff.wait(UINT32_MAX);
          cmdBuff.resetFence();
        }
      }
  }
}
