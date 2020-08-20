#include "LavaCake/Framework.h"


using namespace LavaCake;

int main() {

	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Post Process", 0, 0, 512, 512);

	Framework::Device* d = Framework::Device::getDevice();
	d->initDevices(1, 1, w.m_windowParams);
	d->prepareFrames(nbFrames);

	//texture map
	Framework::TextureBuffer* input = new Framework::TextureBuffer("Data/Textures/mandrill.png", 4);
	input->allocate();


	Framework::StorageImage* output_pass1 = new Framework::StorageImage(input->width(), input->height(), uint32_t(2), VK_FORMAT_R8_UNORM);
	output_pass1->allocate();

	Framework::StorageImage* output_pass2 = new Framework::StorageImage(input->width(), input->height(), uint32_t(2), VK_FORMAT_R8_UNORM);
	output_pass2->allocate();

	Framework::UniformBuffer* sizeBuffer = new Framework::UniformBuffer();
	sizeBuffer->addVariable("width", input->width());
	sizeBuffer->addVariable("height", input->height());
	sizeBuffer->end();

	//pass1 
	Framework::ComputePipeline* computePipeline1 = new Framework::ComputePipeline();

	Framework::ComputeShaderModule* computeFourier1 = new Framework::ComputeShaderModule("Data/Shaders/FourierTransform/fourier_pass1.comp.spv");
	computePipeline1->setComputeShader(computeFourier1);

	computePipeline1->addTextureBuffer(input, VK_SHADER_STAGE_COMPUTE_BIT, 0);
	computePipeline1->addStorageImage(output_pass1, VK_SHADER_STAGE_COMPUTE_BIT, 1);
	computePipeline1->addUniformBuffer(sizeBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);

	computePipeline1->compile();

	//pass2
	Framework::ComputePipeline* computePipeline2 = new Framework::ComputePipeline();

	Framework::ComputeShaderModule* computeFourier2 = new Framework::ComputeShaderModule("Data/Shaders/FourierTransform/fourier_pass2.comp.spv");
	computePipeline2->setComputeShader(computeFourier2);

	computePipeline2->addStorageImage(output_pass1, VK_SHADER_STAGE_COMPUTE_BIT, 0);
	computePipeline2->addStorageImage(output_pass2, VK_SHADER_STAGE_COMPUTE_BIT, 1);
	computePipeline2->addUniformBuffer(sizeBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);

	computePipeline2->compile();

	

	//computePipeline->compute(, input->width(), input->height(), 1);

  return 0;
}