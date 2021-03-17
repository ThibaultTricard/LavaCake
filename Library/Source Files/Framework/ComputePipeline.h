#pragma once

#include "AllHeaders.h"
#include "Pipeline.h"

namespace LavaCake {
	namespace Framework {

  /**
   Class ComputePipeline :
   \brief Helps manage a compute pipeline and it's binding
   Inherit the pipeline Pipeline class
  */
		class ComputePipeline : public Pipeline{

		public : 

			/**
       \brief Default Constructor
			*/
			ComputePipeline() {};

      /**
       \brief set the compute shader module that will be used by the pipeline
       */
			void setComputeModule(ComputeShaderModule* module);
			
			/**
       \brief Compile the pipeline
			*/
			void compile();
			
			virtual void addAttachment(Attachment* a, VkShaderStageFlags stage, int binding = 0) override {};

			/**
			\brief register the execution of the pipeline in  a command buffer
      \param cmdBuff the command buffer in witch the compute pipieline will be registered
      \param dimX : the number of work group for X dimention
      \param dimY : the number of work group for Y dimention
      \param dimZ : the number of work group for Z dimention
			*/
			void compute(CommandBuffer& cmdBuff, uint32_t dimX, uint32_t dimY, uint32_t dimZ);

			~ComputePipeline() {
			
			}
		private :
			
			ComputeShaderModule*																	m_computeModule = NULL;

		};
	}
}
