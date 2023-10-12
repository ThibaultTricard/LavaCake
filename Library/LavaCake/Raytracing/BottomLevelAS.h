#pragma once
#include <LavaCake/Framework/VertexBuffer.h>
#include <LavaCake/Framework/ErrorCheck.h>

namespace LavaCake {
	namespace RayTracing {

		

		class BottomLevelAccelerationStructure {
		public :
			BottomLevelAccelerationStructure() {};

			void addVertexBuffer(std::shared_ptr<Framework::VertexBuffer> vertexBuffer, std::shared_ptr < Framework::Buffer> transformBuffer, bool opaque = true);

			void allocate(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff, bool allowUpdate = false);

			const VkAccelerationStructureKHR& getHandle() const;

			uint64_t getDeviceAddress() const;

			uint32_t getPrimitiveNumber() const;

		private:

			uint32_t m_primCount = 0;
			uint32_t max_primCount = 0;
			uint32_t m_numTriangle = 0;
			

			std::shared_ptr<Framework::Buffer> m_ASBuffer;
			VkAccelerationStructureKHR m_accelerationStructure{};
			uint64_t m_deviceAddress;
			VkAccelerationStructureDeviceAddressInfoKHR m_accelerationDeviceAddressInfo{};
			std::shared_ptr < Framework::Buffer>														m_scratchBuffer;
			std::vector<VkAccelerationStructureGeometryKHR>									m_geometry;

		};

	}
}