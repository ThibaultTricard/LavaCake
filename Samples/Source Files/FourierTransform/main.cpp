#include "LavaCake/Framework.h"


using namespace LavaCake;

int main() {

	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Post Process", 0, 0, 512, 512);

	Framework::Device* d = Framework::Device::getDevice();
	d->initDevices(1, 1, w.m_windowParams);
	d->prepareFrames(nbFrames);


	//PostProcessQuad
	Helpers::Mesh::Mesh* quad = new Helpers::Mesh::Mesh();
	Helpers::Mesh::preparePostProcessQuad(*quad,true);
	Framework::VertexBuffer* quad_vertex_buffer = new Framework::VertexBuffer({ quad }, { 3,2 });
	quad_vertex_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().commandBuffer);

	//texture map
	Framework::TextureBuffer* input = new Framework::TextureBuffer("Data/Textures/mandrill.png", 4);
	input->allocate();


	Framework::TexelBuffer* output_pass1 = new Framework::TexelBuffer();
	std::vector<float> rawdata = std::vector<float>(input->width() * input->height() * 2);
	output_pass1->allocate(rawdata, uint32_t(1));

	Framework::TexelBuffer* output_pass2 = new Framework::TexelBuffer();
	output_pass2->allocate(rawdata, uint32_t(1));

	Framework::UniformBuffer* sizeBuffer = new Framework::UniformBuffer();
	sizeBuffer->addVariable("width", input->width());
	sizeBuffer->addVariable("height", input->height());
	sizeBuffer->end();

	//pass1 
	Framework::ComputePipeline* computePipeline1 = new Framework::ComputePipeline();

	Framework::ComputeShaderModule* computeFourier1 = new Framework::ComputeShaderModule("Data/Shaders/FourierTransform/fourier_pass1.comp.spv");
	computePipeline1->setComputeShader(computeFourier1);

	computePipeline1->addTextureBuffer(input, VK_SHADER_STAGE_COMPUTE_BIT, 0);
	computePipeline1->addTexelBuffer(output_pass1, VK_SHADER_STAGE_COMPUTE_BIT, 1);
	computePipeline1->addUniformBuffer(sizeBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);

	computePipeline1->compile();

	//pass2
	Framework::ComputePipeline* computePipeline2 = new Framework::ComputePipeline();

	Framework::ComputeShaderModule* computeFourier2 = new Framework::ComputeShaderModule("Data/Shaders/FourierTransform/fourier_pass2.comp.spv");
	computePipeline2->setComputeShader(computeFourier2);

	computePipeline2->addTexelBuffer(output_pass1, VK_SHADER_STAGE_COMPUTE_BIT, 0);
	computePipeline2->addTexelBuffer(output_pass2, VK_SHADER_STAGE_COMPUTE_BIT, 1);
	computePipeline2->addUniformBuffer(sizeBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);

	computePipeline2->compile();

	


	//renderPass
	Framework::RenderPass* showPass = new Framework::RenderPass();

	Framework::GraphicPipeline* pipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });
	Framework::VertexShaderModule* vertexShader = new Framework::VertexShaderModule("Data/Shaders/FourierTransform/shader.vert.spv");
	Framework::FragmentShaderModule* fragmentShader = new Framework::FragmentShaderModule("Data/Shaders/FourierTransform/shader.frag.spv");
	pipeline->setVextexShader(vertexShader);
	pipeline->setFragmentModule(fragmentShader);
	pipeline->setVeritices(quad_vertex_buffer);
	pipeline->addTexelBuffer(output_pass2, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	pipeline->addUniformBuffer(sizeBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 1);


	showPass->addSubPass({ pipeline }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);


	showPass->compile();


	VkDevice logical = d->getLogicalDevice();
	VkQueue& compute_queue = d->getComputeQueue(0)->getHandle();
	Buffer::FrameResources& frame = d->getFrameRessources()->at(0);
	VkCommandBuffer commandbuffer = frame.commandBuffer;

	InitVkDestroyer(logical, frame.framebuffer);
	if (!Command::BeginCommandBufferRecordingOperation(commandbuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
		
	}


	sizeBuffer->update(commandbuffer);

	computePipeline1->compute(commandbuffer, input->width(), input->height(), 1);



	VkSemaphoreCreateInfo semaphoreInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	VkSemaphore semaphorePass1;
	VkResult r = LavaCake::vkCreateSemaphore(logical, &semaphoreInfo, nullptr,&semaphorePass1);
	std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
	wait_semaphore_infos.push_back({
		semaphorePass1,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		});


	if (!LavaCake::Command::EndCommandBufferRecordingOperation(commandbuffer)) {

	}

	if (!Command::SubmitCommandBuffersToQueue(compute_queue, {}, { commandbuffer }, { semaphorePass1 }, {})) {
	}


	InitVkDestroyer(logical, frame.framebuffer);
	if (!Command::BeginCommandBufferRecordingOperation(commandbuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
		
	}


	sizeBuffer->update(commandbuffer);

	computePipeline1->compute(commandbuffer, input->width(), input->height(), 1);



	VkSemaphoreCreateInfo semaphoreInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};

	VkSemaphore semaphorePass1;
	VkResult r = LavaCake::vkCreateSemaphore(logical, &semaphoreInfo, nullptr,&semaphorePass1);
	


	if (!LavaCake::Command::EndCommandBufferRecordingOperation(commandbuffer)) {

	}

	if (!Command::SubmitCommandBuffersToQueue(compute_queue, {}, { commandbuffer }, { semaphorePass1 }, {})) {
	}

	std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
	wait_semaphore_infos.push_back({
		semaphorePass1,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		});


	w.Show();


  return 0;
}