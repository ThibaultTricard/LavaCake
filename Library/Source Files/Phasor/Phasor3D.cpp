#include "Phasor3D.h"
#include <stdlib.h>     /* srand, rand */

#ifdef __APPLE__
std::string phasorModulePath = "../LavaCakeShaders";
#else
std::string phasorModulePath = "LavaCakeShaders";
#endif



using namespace LavaCake::Helpers;

namespace LavaCake {
  namespace Phasor {

    Phasor3D::Phasor3D(Helpers::ABBox<3> dim, Helpers::Field3D<float>* F, float Fmin, Helpers::Field3D<vec3f>* D, uint32_t seed) {
      vec3f diag = dim.diag() * Fmin;

      m_cellsize = 1.0f / Fmin;

      m_cellsDim = vec3u({ uint32_t(diag[0]),uint32_t(diag[1]), uint32_t(diag[2]) });

      if (float(m_cellsDim[0]) - diag[0] > 0) {
        m_cellsDim[0]++;
      }
      if (float(m_cellsDim[1]) - diag[1] > 0) {
        m_cellsDim[1]++;
      }
      if (float(m_cellsDim[2]) - diag[2] > 0) {
        m_cellsDim[2]++;
      }


      m_tightBoundingBox = Helpers::ABBox<3>(dim.A(), dim.A() + vec3f({ float(m_cellsDim[0]), float(m_cellsDim[1]),float(m_cellsDim[2])}) * m_cellsize);

      m_kernelBoundingBox = Helpers::ABBox<3>(dim.A() - vec3f({ 2.0f,2.0f,2.0f }) * m_cellsize, dim.A() + vec3f({ float(m_cellsDim[0] + 2), float(m_cellsDim[1] + 2),float(m_cellsDim[2] + 2) }) * m_cellsize);

      //we add two cells of margin on each direction to
      m_cellsDim[0] += 4;
      m_cellsDim[1] += 4;
      m_cellsDim[2] += 4;

      std::srand(seed);

      m_D = D;
      m_F = F;

      m_cells = std::vector<phasor3DCell>(m_cellsDim[0] * m_cellsDim[1]* m_cellsDim[2]);

      for (uint32_t i = 0; i < m_cellsDim[0]; i++) {

        for (uint32_t j = 0; j < m_cellsDim[1]; j++) {

          for (uint32_t k = 0; k < m_cellsDim[2]; k++) {
            uint32_t index = i + j * m_cellsDim[0] + k* m_cellsDim[0]* m_cellsDim[1];

            for (uint32_t l = 0; l < 8; l++) {
              phasor3DKernel kernel;

              //we rand
              kernel.pos = vec3f({ float(rand()) / float(RAND_MAX)  ,float(rand()) / float(RAND_MAX),float(rand()) / float(RAND_MAX) });

              vec3f pos = m_kernelBoundingBox.A() + (vec3f({ float(i),float(j),float(k) }) + kernel.pos) * m_cellsize;

              kernel.f = F->sample(pos) / Fmin;
              auto dir = D->sample(pos);
              kernel.direction = dir / sqrt((dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]));
              kernel.phase = 0.0f;

              m_cells[index].kernels[l] = kernel;

            }
          }



        }
      }




    }


    void Phasor3D::init(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff) {

      m_kernelBuffer = new Framework::Buffer();
      m_kernelBuffer->allocate(queue, cmdBuff, m_cells, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT );

      m_optimisationModule = new Framework::ComputeShaderModule(phasorModulePath + "/Phasor/optimisationModule3D.comp.spv");

      m_optimisationPipeline = new Framework::ComputePipeline();
      m_optimisationPipeline->setComputeModule(m_optimisationModule);

      m_optimisationBuffer = new Framework::UniformBuffer();
      m_optimisationBuffer->addVariable("GridSize", vec4u({ m_cellsDim[0],m_cellsDim[1],m_cellsDim[2],0 }));
      m_optimisationBuffer->end();

      m_optimisationPipeline->addTexelBuffer(m_kernelBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0);
      m_optimisationPipeline->addUniformBuffer(m_optimisationBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 1);

      m_optimisationPipeline->compile();


      m_samplingUniformBuffer = new Framework::UniformBuffer();
      m_samplingUniformBuffer->addVariable("cellSize", vec4u({ m_cellsDim[0],m_cellsDim[1],m_cellsDim[2],0 }));
      m_samplingUniformBuffer->addVariable("resultSize", vec4u({ 0,0,0,0 }));
      m_samplingUniformBuffer->addVariable("sampleMin", vec4f({ 0,0,0,0 }));
      m_samplingUniformBuffer->addVariable("sampleMax", vec4f({ 0,0,0,0 }));
      m_samplingUniformBuffer->addVariable("gridMin", vec4f({ m_kernelBoundingBox.A()[0],m_kernelBoundingBox.A()[1],m_kernelBoundingBox.A()[2], 0}));
      m_samplingUniformBuffer->addVariable("gridMax", vec4f({ m_kernelBoundingBox.B()[0],m_kernelBoundingBox.B()[1],m_kernelBoundingBox.B()[2], 0 }));
      m_samplingUniformBuffer->addVariable("singleCellSize_mm", vec4f({m_cellsize,0.0f,0.0f,0.0f}));
      m_samplingUniformBuffer->end();


      m_samplingModule = new Framework::ComputeShaderModule(phasorModulePath + "/Phasor/samplingModule3D.comp.spv");

    }


    void Phasor3D::phaseOptimisation(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, uint32_t nbOptimisation) {
      for (int i = 0; i < nbOptimisation; i++) {
        cmdBuff.resetFence();
        cmdBuff.beginRecord();
        m_optimisationBuffer->update(cmdBuff);
        m_optimisationPipeline->compute(cmdBuff, m_cellsDim[0] * m_cellsDim[1]* m_cellsDim[2], 1, 1);
        cmdBuff.endRecord();
        cmdBuff.submit(queue, {}, {});
        cmdBuff.wait(UINT32_MAX);
      }
      cmdBuff.resetFence();
    }


    void Phasor3D::sample(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, Helpers::ABBox<3> sampleBoundingBox, vec3u sampleResolution) {

      if (m_samplingBuffer != nullptr) {
        delete m_samplingBuffer;
      }
      if (m_samplingPipeline != nullptr) {
        delete m_samplingPipeline;
      }

      m_samplingBuffer = new Framework::Buffer();
      m_samplingBuffer->allocate(sampleResolution[0] * sampleResolution[1] * sampleResolution[2] * sizeof(float), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

      std::vector<float> f(sampleResolution[0] * sampleResolution[1] * sampleResolution[2]);
      std::vector<vec4f> dir(sampleResolution[0] * sampleResolution[1] * sampleResolution[2]);
      std::vector<float> div(sampleResolution[0] * sampleResolution[1] * sampleResolution[2]);

      for (int i = 0; i < sampleResolution[0]; i++) {
        for (int j = 0; j < sampleResolution[1]; j++) {
          for (int k = 0; k < sampleResolution[2]; k++) {
            vec3f pos = sampleBoundingBox.A() + vec3f({ float(i) / float(sampleResolution[0]),float(j) / float(sampleResolution[1]),float(k) / float(sampleResolution[2]) }) * sampleBoundingBox.diag();

            f[i + j * sampleResolution[0] + k * sampleResolution[0] * sampleResolution[1]] = m_F->sample(pos);
            auto d = m_D->sample(pos);
            
            dir[i + j * sampleResolution[0] + k * sampleResolution[0] * sampleResolution[1]][0] = d[0];
            dir[i + j * sampleResolution[0] + k * sampleResolution[0] * sampleResolution[1]][1] = d[1];
            dir[i + j * sampleResolution[0] + k * sampleResolution[0] * sampleResolution[1]][2] = d[2];
            
            div[i + j * sampleResolution[0] + k * sampleResolution[0] * sampleResolution[1]] = 0.0f;
          }
        }
      }
      cmdBuff.wait(UINT32_MAX);
      cmdBuff.resetFence();
      Framework::Buffer* dirBuffer = new Framework::Buffer();
      dirBuffer->allocate(queue, cmdBuff, dir, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_FORMAT_R32G32B32A32_SFLOAT);

      Framework::Buffer* fBuffer = new Framework::Buffer();
      fBuffer->allocate(queue, cmdBuff, f, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

      Framework::Buffer* divBuffer = new Framework::Buffer();
      divBuffer->allocate(queue, cmdBuff, div, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

      m_samplingUniformBuffer->setVariable("resultSize", vec4u({ sampleResolution[0],sampleResolution[1],sampleResolution[2],0 }));
      m_samplingUniformBuffer->setVariable("sampleMin",  vec4f({ sampleBoundingBox.A()[0],sampleBoundingBox.A()[1],sampleBoundingBox.A()[2],0.0f }));
      m_samplingUniformBuffer->setVariable("sampleMax",  vec4f({ sampleBoundingBox.B()[0],sampleBoundingBox.B()[1],sampleBoundingBox.B()[2],0.0f }));

      m_samplingPipeline = new Framework::ComputePipeline();

      m_samplingPipeline->setComputeModule(m_samplingModule);

      m_samplingPipeline->addTexelBuffer(m_kernelBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 1);
      m_samplingPipeline->addUniformBuffer(m_samplingUniformBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);
      m_samplingPipeline->addTexelBuffer(fBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 3);
      m_samplingPipeline->addTexelBuffer(dirBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 4);
      m_samplingPipeline->addTexelBuffer(divBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 5);
      m_samplingPipeline->addTexelBuffer(m_samplingBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 6);
      m_samplingPipeline->compile();

      cmdBuff.resetFence();
      cmdBuff.beginRecord();

      m_samplingUniformBuffer->update(cmdBuff);
      m_samplingPipeline->compute(cmdBuff, sampleResolution[0]/16+1, sampleResolution[1], sampleResolution[2]);

      cmdBuff.endRecord();
      cmdBuff.submit(queue, {}, {});
      cmdBuff.wait(UINT32_MAX);
      cmdBuff.resetFence();

      delete fBuffer;
      delete dirBuffer;
      delete divBuffer;
    }



  }
}
