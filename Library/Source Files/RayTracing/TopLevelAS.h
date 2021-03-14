#ifdef RAYTRACING
#pragma once
#include "AllHeaders.h"
#include "BottomLevelAS.h"



namespace LavaCake {
  namespace RayTracing {
    
    class TopLevelAS {
    public:
      void addInstance(BottomLevelAS* bottomLevelAS, VkTransformMatrixKHR& transform, uint32_t instanceID, uint32_t hitGroupOffset);

      void alloctate(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate = false);

      VkAccelerationStructureKHR& getHandle() {
        return m_accelerationStructure;
      }

      void createAccelerationStructure(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);

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
      VkAccelerationStructureBuildGeometryInfoKHR m_accelerationBuildGeometryInfo = {};
      Framework::Buffer* m_ASBuffer = nullptr;
      Framework::Buffer m_scratchBuffer;
      std::vector<Framework::Buffer> m_instancesBuffers;
		};
	}
}
#endif