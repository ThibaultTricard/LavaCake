#include "Framework/Framework.h"


using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;
using namespace LavaCake::Core;

int main() {

	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Fourier Transform", 512, 512);

	Framework::Device* d = Framework::Device::getDevice();
	d->initDevices(1, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init();

	Framework::Queue* queue = d->getGraphicQueue(0);
	VkQueue& present_queue = d->getPresentQueue()->getHandle();

	VkQueue& compute_queue = d->getComputeQueue(0)->getHandle();

	VkDevice logical = d->getLogicalDevice();
	VkExtent2D size = s->size();

	std::vector<Framework::CommandBuffer> commandBuffer = std::vector<Framework::CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
		commandBuffer[i].addSemaphore();
	}

	//PostProcessQuad
	Geometry::Mesh_t* quad = new Geometry::IndexedMesh<Geometry::TRIANGLE>(Geometry::P3UV);

	quad->appendVertex({ -1.0,-1.0,0.0,0.0,0.0 });
	quad->appendVertex({ -1.0, 1.0,0.0,0.0,1.0 });
	quad->appendVertex({ 1.0, 1.0,0.0,1.0,1.0 });
	quad->appendVertex({ 1.0,-1.0,0.0,1.0,0.0 });

	quad->appendIndex(0);
	quad->appendIndex(1);
	quad->appendIndex(2);

	quad->appendIndex(2);
	quad->appendIndex(3);
	quad->appendIndex(0);

	Framework::VertexBuffer* quad_vertex_buffer = new Framework::VertexBuffer({ quad });
	quad_vertex_buffer->allocate(queue, commandBuffer[0]);

	//texture map
	Framework::TextureBuffer* input = new Framework::TextureBuffer("Data/Textures/mandrill.png", 4);
	input->allocate(queue, commandBuffer[0]);


	Framework::Buffer* output_pass1 = new Framework::Buffer();
	std::vector<float> rawdata = std::vector<float>(input->width() * input->height() * uint32_t(2));
	output_pass1->allocate(queue, commandBuffer[0],rawdata, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

	Framework::Buffer* output_pass2 = new Framework::Buffer();
	output_pass2->allocate(queue, commandBuffer[0], rawdata, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

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

	Framework::GraphicPipeline* pipeline = new Framework::GraphicPipeline(vec3f({ 0,0,0 }) , vec3f({ float(size.width),float(size.height),1.0f }) , vec2f({ 0,0 }) , vec2f({ float(size.width),float(size.height) }));
	Framework::VertexShaderModule* vertexShader = new Framework::VertexShaderModule("Data/Shaders/FourierTransform/shader.vert.spv");
	Framework::FragmentShaderModule* fragmentShader = new Framework::FragmentShaderModule("Data/Shaders/FourierTransform/shader.frag.spv");
	pipeline->setVextexShader(vertexShader);
	pipeline->setFragmentModule(fragmentShader);
	pipeline->setVertices(quad_vertex_buffer);
	pipeline->addTexelBuffer(output_pass2, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	pipeline->addUniformBuffer(sizeBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	showPass->addSubPass({ pipeline }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);

	showPass->compile();



	std::vector<Framework::FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new Framework::FrameBuffer(s->size().width, s->size().height));
		showPass->prepareOutputFrameBuffer(*frameBuffers[i]);
	}


	commandBuffer[0].wait(2000000000);
	commandBuffer[0].resetFence();
	commandBuffer[0].beginRecord();

	sizeBuffer->update(commandBuffer[0].getHandle());

	computePipeline1->compute(commandBuffer[0].getHandle(), input->width(), input->height(), 1);

	
	computePipeline2->compute(commandBuffer[0].getHandle(), input->width(), input->height(), 1);


	commandBuffer[0].endRecord();


	if (!SubmitCommandBuffersToQueue(compute_queue, {}, { commandBuffer[0].getHandle() }, { commandBuffer[0].getSemaphore(1) }, { commandBuffer[0].getFence()})) {
	}




	int f = 0;
	while (w.running()) {
		w.updateInput();
		f++;
		f = f % nbFrames;


		Framework::SwapChainImage& image = s->AcquireImage();

		std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});


		commandBuffer[f].wait(2000000000);
		commandBuffer[f].resetFence();
		commandBuffer[f].beginRecord();


		showPass->setSwapChainImage(*frameBuffers[f], image);


		showPass->draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), vec2u({ 0,0 }), vec2u({ size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });

		commandBuffer[f].endRecord();


		if (!SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(0) }, commandBuffer[f].getFence())) {
			continue;
		}

		PresentInfo present_info = {
			s->getHandle(),                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                                   // uint32_t               ImageIndex
		};
		if (!PresentImage(present_queue, { commandBuffer[f].getSemaphore(0) }, { present_info })) {
			continue;
		}
	}

	d->end();


  return 0;
}