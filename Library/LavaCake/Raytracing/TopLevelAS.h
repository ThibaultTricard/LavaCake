#pragma once
#include "AllHeaders.h"
#include "BottomLevelAS.h"



namespace LavaCake {
  namespace RayTracing {
    
    class TopLevelAccelerationStructure {
    public:
      void addInstance(BottomLevelAccelerationStructure* bottomLevelAS, VkTransformMatrixKHR& transform, uint32_t instanceID, uint32_t hitGroupOffset);

      void alloctate(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate = false);

      const VkAccelerationStructureKHR& getHandle() const{
        return m_accelerationStructure;
      }

      void createAccelerationStructure(VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);

		private : 

      struct instance{
        BottomLevelAccelerationStructure* bottomLevelAS;
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
      VkAccelerationStructureBuildGeometryInfoKHR m_accelerationBuildGeometryInfo = {};
      std::shared_ptr < Framework::Buffer> m_ASBuffer = nullptr;
      std::shared_ptr < Framework::Buffer> m_scratchBuffer;
      std::vector<std::shared_ptr<Framework::Buffer>> m_instancesBuffers;
		};
	}
}