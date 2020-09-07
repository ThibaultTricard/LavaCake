#pragma once

#include "AllHeaders.h"
#include "Pipeline.h"

namespace LavaCake {
	namespace Framework {

		class ComputePipeline : public Pipeline{

		public : 

			/**
			*Constructor of the class
			*/
			ComputePipeline() {};

			void setComputeShader(ComputeShaderModule* module);
			
			/**
			*Prepare the Compute pipeline for it's usage
			*Warning : If a modification is made to the compute pipeline after this operation,
			*The compute pipeline need to be compiled again before being used
			*/
			void compile();
			
			/**
			*add an attachment to the pipeline and scpecify it's binding and shader stage
			*/
			virtual void addAttachment(Attachment* a, VkShaderStageFlags stage, int binding = 0) override {};

			/**
			*dispach the compute shader
			*/
			void compute(const VkCommandBuffer buffer, uint32_t dimX, uint32_t dimY, uint32_t dimZ);

			~ComputePipeline() {
			
			}
		private :
			
			ComputeShaderModule*																	m_computeModule = NULL;

		};
	}
}