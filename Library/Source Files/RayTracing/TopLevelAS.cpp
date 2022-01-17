#include "TopLevelAS.h"

namespace LavaCake {
  namespace RayTracing {

     
      void TopLevelAccelerationStructure::addInstance(BottomLevelAccelerationStructure* bottomLevelAS, VkTransformMatrixKHR& transform, uint32_t instanceID, uint32_t hitGroupOffset) {
        m_instances.push_back({ bottomLevelAS ,transform , instanceID , hitGroupOffset });

        VkAccelerationStructureInstanceKHR instance{};
        instance.transform = transform;
        instance.instanceCustomIndex = instanceID;
        instance.mask = 0xFF;
        instance.instanceShaderBindingTableRecordOffset = hitGroupOffset;
        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instance.accelerationStructureReference = bottomLevelAS->getDeviceAddress();
        m_AccelerationStructureInstances.push_back(instance);

      }

      void TopLevelAccelerationStructure::alloctate(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate) {
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkPhysicalDevice phyDevice = d->getPhysicalDevice();
        m_instancesBuffers = std::vector<Framework::Buffer>(m_AccelerationStructureInstances.size());
        std::vector<VkAccelerationStructureGeometryKHR> accelerationStructureGeometry;
        uint32_t primitive_count = 0;
        for (int i = 0; i < m_AccelerationStructureInstances.size(); i++) {
          // Buffer for instance data
          std::vector<VkAccelerationStructureInstanceKHR> instance = { m_AccelerationStructureInstances[i] };

          m_instancesBuffers[i].allocate(queue,
            cmdBuff,
            instance,
            VkBufferUsageFlagBits(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR),
            VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

          VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
          instanceDataDeviceAddress.deviceAddress = m_instancesBuffers[i].getBufferDeviceAddress();

          VkAccelerationStructureGeometryKHR asg{};
          asg.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
          asg.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
          asg.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
          asg.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
          asg.geometry.instances.arrayOfPointers = VK_FALSE;
          asg.geometry.instances.data = instanceDataDeviceAddress;

          accelerationStructureGeometry.push_back(asg);

          primitive_count += m_instances[i].bottomLevelAS->getPrimitiveNumber();
          m_primitive_count.push_back(m_instances[i].bottomLevelAS->getPrimitiveNumber());
        }

        // Get size info
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.geometryCount = (uint32_t)accelerationStructureGeometry.size();
        accelerationStructureBuildGeometryInfo.pGeometries = accelerationStructureGeometry.data();

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
        m_scratchBuffer.allocate(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        VkBufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
        scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        scratchBufferDeviceAddressInfo.buffer = m_scratchBuffer.getHandle();


        m_accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        m_accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        m_accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        m_accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        m_accelerationBuildGeometryInfo.dstAccelerationStructure = m_accelerationStructure;
        m_accelerationBuildGeometryInfo.geometryCount = (uint32_t)accelerationStructureGeometry.size();
        m_accelerationBuildGeometryInfo.pGeometries = accelerationStructureGeometry.data();
        m_accelerationBuildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddressKHR(logical, &scratchBufferDeviceAddressInfo);

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = primitive_count;
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
            &m_accelerationBuildGeometryInfo,
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
            &m_accelerationBuildGeometryInfo,
            accelerationBuildStructureRangeInfos.data());
          cmdBuff.endRecord();

          cmdBuff.submit(queue, {}, {});

          cmdBuff.wait(UINT32_MAX);
          cmdBuff.resetFence();
        }


      }

      void TopLevelAccelerationStructure::createAccelerationStructure(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
      {
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        m_ASBuffer = new Framework::Buffer();
        m_ASBuffer->allocate(buildSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
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
  }
}