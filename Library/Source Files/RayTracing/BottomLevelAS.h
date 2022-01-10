#pragma once
#ifdef RAYTRACING
#include "Framework/VertexBuffer.h"
#include "Framework/ErrorCheck.h"

namespace LavaCake {
	namespace RayTracing {

		

		class BottomLevelAccelerationStructure {
		public :
			BottomLevelAccelerationStructure() {};

			void addVertexBuffer(Framework::VertexBuffer* vertexBuffer, Framework::TransformBuffer* transformBuffer, bool opaque = true);

			void allocate(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate = false);

			VkAccelerationStructureKHR& getHandle();

			uint64_t getDeviceAddress();

			uint32_t getPrimitiveNumber();

		private:

			uint32_t m_primCount = 0;
			uint32_t m_numTriangle = 0;

			Framework::Buffer m_ASBuffer;
			VkAccelerationStructureKHR m_accelerationStructure{};
			uint64_t m_deviceAddress;
			VkAccelerationStructureDeviceAddressInfoKHR m_accelerationDeviceAddressInfo{};
			Framework::Buffer														m_scratchBuffer;
			std::vector<VkAccelerationStructureGeometryKHR> m_geometry;

		};

	}
}
#endif 