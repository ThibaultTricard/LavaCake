#include "Framework/Framework.h"

using namespace LavaCake::Framework;

int main() {

	Window w("Lavacake: JumpFlooding", 512, 512);
	Device* d = Device::getDevice();
	d->initDevices(1, 1, w.m_windowParams);
	SwapChain* s = SwapChain::getSwapChain();
	s->init();
	GraphicQueue* queue = d->getGraphicQueue(0);

	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	VkQueue& compute_queue = d->getComputeQueue(0)->getHandle();
	CommandBuffer* cmbBuff = new CommandBuffer();
	cmbBuff->addSemaphore();

	ComputePipeline* jumpFloodPipeline = new ComputePipeline();
	VkExtent2D size = s->size();
	TexelBuffer* seeds = new TexelBuffer();
	std::vector<float> data(512 * 512 * 4);

	for (uint32_t i = 0; i < data.size(); i++) {
		data[i] = -512.0f;
	}

	int nseeds = 200;

	//setting up seeds
	for (int i = 0; i < nseeds; i++) {
		int x = float(std::rand()) / float(RAND_MAX) * 512;
		int y = float(std::rand()) / float(RAND_MAX) * 512;

		data[(x + y * 512) * 4] = x;
		data[(x + y * 512) * 4 + 1] = y;
		data[(x + y * 512) * 4 + 2] = float(i)/ nseeds;
		data[(x + y * 512) * 4 + 3] = 0.0;
	}

	seeds->allocate(queue->getHandle(), cmbBuff->getHandle(), data, uint32_t(4));

	jumpFloodPipeline->addTexelBuffer(seeds, VK_SHADER_STAGE_COMPUTE_BIT, 0);

	ComputeShaderModule* compute = new ComputeShaderModule("Data/Shaders/JumpFlooding/JumpFlooding.comp.spv");
	jumpFloodPipeline->setComputeShader(compute);
	UniformBuffer* passNumber = new UniformBuffer();
	LavaCake::Data* dimention = new LavaCake::vec2i({ 512,512 });
	passNumber->addVariable("dimention", dimention);
	passNumber->addVariable("passNumber", unsigned int(0));
	passNumber->end();
	jumpFloodPipeline->addUniformBuffer(passNumber, VK_SHADER_STAGE_COMPUTE_BIT, 1);
	jumpFloodPipeline->compile();



	//PostProcessQuad
	LavaCake::Helpers::Mesh::Mesh* quad = new LavaCake::Helpers::Mesh::Mesh();
	LavaCake::Helpers::Mesh::preparePostProcessQuad(*quad, true);
	VertexBuffer* quad_vertex_buffer = new VertexBuffer({ quad }, { 3,2 });
	quad_vertex_buffer->allocate(queue->getHandle(), cmbBuff->getHandle());

	//renderPass
	RenderPass* showPass = new RenderPass();

	GraphicPipeline* pipeline = new GraphicPipeline(LavaCake::vec3f({ 0,0,0 }), LavaCake::vec3f({ float(size.width),float(size.height),1.0f }), LavaCake::vec2f({ 0,0 }), LavaCake::vec2f({ float(size.width),float(size.height) }));
	VertexShaderModule* vertexShader = new VertexShaderModule("Data/Shaders/JumpFlooding/shader.vert.spv");
	FragmentShaderModule* fragmentShader = new FragmentShaderModule("Data/Shaders/JumpFlooding/shader.frag.spv");
	pipeline->setVextexShader(vertexShader);
	pipeline->setFragmentModule(fragmentShader);
	pipeline->setVeritices(quad_vertex_buffer);
	pipeline->addTexelBuffer(seeds, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	pipeline->addUniformBuffer(passNumber, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	showPass->addSubPass({ pipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);

	showPass->compile();

	FrameBuffer* frameBuffer = new FrameBuffer(s->size().width, s->size().height);
	showPass->prepareOutputFrameBuffer(*frameBuffer);



	

	int pass = 0;
	while (w.running()) {
		w.updateInput();
		
		cmbBuff->wait(2000000000);
		cmbBuff->resetFence();
		cmbBuff->beginRecord();
		passNumber->setVariable("passNumber", pass + 1);
		passNumber->update(cmbBuff->getHandle());
		jumpFloodPipeline->compute(cmbBuff->getHandle(), 512, 512, 1);
		cmbBuff->endRecord();
		if (!LavaCake::Command::SubmitCommandBuffersToQueue(compute_queue, {}, { cmbBuff->getHandle() }, {  }, cmbBuff->getFence())) {

		}
		pass++;
		SwapChainImage& image = s->AcquireImage();

		std::vector<LavaCake::Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});


		cmbBuff->wait(2000000000);
		cmbBuff->resetFence();
		cmbBuff->beginRecord();


		showPass->setSwapChainImage(*frameBuffer, image);


		showPass->draw(cmbBuff->getHandle(), frameBuffer->getHandle(), LavaCake::vec2u({ 0,0 }), LavaCake::vec2u({ size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });

		cmbBuff->endRecord();


		if (!LavaCake::Command::SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { cmbBuff->getHandle() }, { cmbBuff->getSemaphore(0) }, cmbBuff->getFence())) {
			continue;
		}

		LavaCake::Presentation::PresentInfo present_info = {
			s->getHandle(),                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                                   // uint32_t               ImageIndex
		};
		if (!LavaCake::Presentation::PresentImage(present_queue, { cmbBuff->getSemaphore(0) }, { present_info })) {
			continue;
		}

		Sleep(500);
	}
	d->end();
	delete cmbBuff;
	delete jumpFloodPipeline;
	delete compute;
	delete seeds;
	delete showPass;
	delete vertexShader;
	delete fragmentShader;
	delete pipeline;
	delete frameBuffer;
}