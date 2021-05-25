#include "Phasor2D.h"
#include <stdlib.h>     /* srand, rand */

#ifdef __APPLE__
std::string phasorModulePath ="../LavaCakeShaders";
#else
std::string phasorModulePath ="LavaCakeShaders";
#endif



using namespace LavaCake::Helpers;

namespace LavaCake {
  namespace Phasor {
  
  Phasor2D::Phasor2D(Helpers::ABBox<2> dim, Helpers::Field2D<float>* F, float Fmin, Helpers::Field2D<vec2f>* D, uint32_t seed){
    vec2f diag = dim.diag() * Fmin;
    
    float cellsize = 1.0f/Fmin;
    
    m_cellsDim = vec2u({uint32_t(diag[0]),uint32_t(diag[1])});
    
    if(float(m_cellsDim[0]) -diag[0] > 0 ){
      m_cellsDim[0]++;
    }
    if(float(m_cellsDim[1]) -diag[1] > 0 ){
      m_cellsDim[1]++;
    }
    
    m_tightBoundingBox = Helpers::ABBox<2>(dim.min(), dim.min() + vec2f({float(m_cellsDim[0]), float(m_cellsDim[1])}) * cellsize);
    
    m_kernelBoundingBox = Helpers::ABBox<2>(dim.min() - vec2f({1.0f,1.0f}) * cellsize, dim.min() + vec2f({float(m_cellsDim[0]+1), float(m_cellsDim[1]+1)}) * cellsize);
    
    //we add on cells of margin on each direction to
    m_cellsDim[0]+=2;
    m_cellsDim[1]+=2;
    
    std::srand(seed);
    
    m_D = D;
    m_F = F;
    
    m_cells = std::vector<phasor2DCell>(m_cellsDim[0] * m_cellsDim[1]);
    
    for(uint32_t i = 0; i <m_cellsDim[0]; i++){
      for(uint32_t j = 0; j <m_cellsDim[1]; j++){
        
        uint32_t index = i + j *m_cellsDim[0];
        
        for(uint32_t k = 0; k < 8; k++){
          phasor2DKernel kernel;
          
          //we rand
          kernel.pos = vec2f({float(rand()) / float(RAND_MAX)  ,float(rand()) / float(RAND_MAX)});
          
          vec2f pos = m_kernelBoundingBox.min() + vec2f({float(i),float(j)}) * cellsize + kernel.pos;
          
          kernel.f =  F->sample(pos) / Fmin;
          kernel.direction =  D->sample(pos);
          kernel.phase = 0.0f;
          
          m_cells[index].kernels[k] = kernel;
          
        }
        
        
        
      }
    }
    
    
    
    
  }
  
  
  void Phasor2D::init(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff){
    
    m_kernelBuffer = new Framework::Buffer();
    m_kernelBuffer->allocate(queue, cmdBuff, m_cells, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    
    m_optimisationModule = new Framework::ComputeShaderModule(phasorModulePath + "/Phasor/optimisationModule2D.comp.spv");
    
    m_optimisationPipeline = new Framework::ComputePipeline();
    m_optimisationPipeline->setComputeModule(m_optimisationModule);
    
    m_optimisationBuffer = new Framework::UniformBuffer();
    m_optimisationBuffer->addVariable("GridSize", m_cellsDim);
    m_optimisationBuffer->end();
    
    m_optimisationPipeline->addTexelBuffer(m_kernelBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0);
    m_optimisationPipeline->addUniformBuffer(m_optimisationBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 1);
    
    m_optimisationPipeline->compile();
    
    
    m_samplingUniformBuffer = new Framework::UniformBuffer();
    m_samplingUniformBuffer->addVariable("cellSize",    m_cellsDim);
    m_samplingUniformBuffer->addVariable("resultSize",  vec2u({0,0}));
    m_samplingUniformBuffer->addVariable("sampleMin",   vec2f({0,0}));
    m_samplingUniformBuffer->addVariable("sampleMax",   vec2f({0,0}));
    m_samplingUniformBuffer->addVariable("gridMin",     m_kernelBoundingBox.min());
    m_samplingUniformBuffer->addVariable("gridMax",     m_kernelBoundingBox.max());
    m_samplingUniformBuffer->end();
    
    
    m_samplingModule = new Framework::ComputeShaderModule(phasorModulePath + "/Phasor/samplingModule2D.comp.spv");
    
  }

  
  void Phasor2D::phaseOptimisation(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff,uint32_t nbOptimisation){
    for (int i = 0; i < nbOptimisation; i++) {
      cmdBuff.resetFence();
      cmdBuff.beginRecord();
      m_optimisationBuffer->update(cmdBuff);
      m_optimisationPipeline->compute(cmdBuff, m_cellsDim[0] * m_cellsDim[1], 1, 1);
      cmdBuff.endRecord();
      cmdBuff.submit(queue, {}, {});
      cmdBuff.wait(UINT32_MAX);
    }
  }
  
  
  void Phasor2D::sample(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, Helpers::ABBox<2> sampleBoundingBox, vec2u sampleResolution){
    
    if(m_samplingBuffer != nullptr){
      delete m_samplingBuffer;
    }
    if(m_samplingPipeline != nullptr){
      delete m_samplingPipeline;
    }
    
    m_samplingBuffer = new Framework::Buffer();
    m_samplingBuffer->allocate(sampleResolution[0] * sampleResolution[1] * sizeof(float), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    
    std::vector<float> f(sampleResolution[0] * sampleResolution[1]);
    std::vector<vec2f> dir(sampleResolution[0] * sampleResolution[1]);
    std::vector<float> div(sampleResolution[0] * sampleResolution[1]);
    
    for(int i = 0; i< sampleResolution[0]; i++){
      for(int j = 0; j< sampleResolution[1]; j++){
        vec2f pos =sampleBoundingBox.min() + vec2f({float(i)/float(sampleResolution[0]),float(j)/float(sampleResolution[1])}) * sampleBoundingBox.diag();
        
        f[i + j * sampleResolution[0]] = m_F->sample(pos);
        dir[i + j * sampleResolution[0]] = m_D->sample(pos);
        div[i + j * sampleResolution[0]] = 0;
        
      }
    }
    
    Framework::Buffer* dirBuffer = new Framework::Buffer();
    dirBuffer->allocate(queue, cmdBuff, dir, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT, VK_FORMAT_R32G32_SFLOAT);
    
    Framework::Buffer* fBuffer = new Framework::Buffer();
    dirBuffer->allocate(queue, cmdBuff, f, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    
    Framework::Buffer* divBuffer = new Framework::Buffer();
    divBuffer->allocate(queue, cmdBuff, div, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    
    m_samplingUniformBuffer->setVariable("resultSize", sampleResolution);
    m_samplingUniformBuffer->setVariable("sampleMin", sampleBoundingBox.min());
    m_samplingUniformBuffer->setVariable("sampleMax", sampleBoundingBox.max());
    
    m_samplingPipeline = new Framework::ComputePipeline();
    
    m_samplingPipeline->setComputeModule(m_samplingModule);
    
    m_samplingPipeline->addBuffer(m_kernelBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 1);
    m_samplingPipeline->addUniformBuffer(m_samplingUniformBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);
    m_samplingPipeline->addBuffer(fBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 3);
    m_samplingPipeline->addBuffer(dirBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 4);
    m_samplingPipeline->addBuffer(divBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 5);
    m_samplingPipeline->addBuffer(m_samplingBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 6);
    m_samplingPipeline->compile();

    cmdBuff.resetFence();
    cmdBuff.beginRecord();
    
    m_samplingUniformBuffer->update(cmdBuff);
    m_samplingPipeline->compute(cmdBuff, sampleResolution[0], sampleResolution[1], 1);
    
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
