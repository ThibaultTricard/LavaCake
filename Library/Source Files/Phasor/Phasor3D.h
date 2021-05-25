#pragma once
#include "AllHeaders.h"
#include "../Helpers/Field.h"
#include "../Framework/Framework.h"

namespace LavaCake {
  namespace Phasor {


    struct phasor3DKernel {
      vec3f pos;
      float f;
      float phase;
      vec3f direction;
    };

    struct phasor3DCell {
      std::array<phasor3DKernel, 8> kernels;
    };

    /**
     *\brief Phasor 2D a helper class to compute optimized phasor noise field in 2D
     */
    class Phasor3D {
    public:
      /**
       *\brief
       *\param dim the size of the domain in mm
       *\param F the frequency field in oscillation/mm
       *\param Fmin minimal frequency of the frequency field
       *\param D the direction field
       *\param seed the seed used for the kernel generation
       */
      Phasor3D(Helpers::ABBox<3> dim, Helpers::Field3D<float>* F, float Fmin, Helpers::Field3D<vec3f>* D, uint32_t seed = 1);


      void init(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff);

      /**
       *\brief execute a predefined number of optimisation step
       *\param nBoptimisation the number of optimisation step
       */
      void phaseOptimisation(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, uint32_t nbOptimisation);

      void sample(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, Helpers::ABBox<3> sampleBoundingBox, vec3u sampleResolution);

      Framework::Buffer* getSampleBuffer() { return m_samplingBuffer; }

    private:
      vec3u                             m_cellsDim;
      std::vector<phasor3DCell>         m_cells;
      float                             m_cellsize;
      Helpers::ABBox<3>                 m_kernelBoundingBox;
      Helpers::ABBox<3>                 m_tightBoundingBox;

      Helpers::Field3D<float>* m_F;
      Helpers::Field3D<vec3f>* m_D;

      Framework::Buffer* m_kernelBuffer;

      Framework::ComputeShaderModule* m_optimisationModule;
      Framework::ComputePipeline* m_optimisationPipeline;
      Framework::UniformBuffer* m_optimisationBuffer;


      Framework::Buffer* m_samplingBuffer = nullptr;
      Framework::ComputeShaderModule* m_samplingModule;
      Framework::ComputePipeline* m_samplingPipeline = nullptr;
      Framework::UniformBuffer* m_samplingUniformBuffer;
    };

  }
}
