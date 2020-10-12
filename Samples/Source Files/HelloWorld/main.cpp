#include "Framework/Framework.h"
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32 true
#include "glfw3.h"
#include "glfw3native.h"

using namespace LavaCake::Framework;
int main() {

	Window w("LavaCake HelloWorld", 512, 512);

	int nbFrames = 3;
	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init();
	VkExtent2D size = s->size();
	VkQueue queue = d->getGraphicQueue(0)->getHandle();
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	std::vector<CommandBuffer> commandBuffer = std::vector<CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
	}

	LavaCake::Helpers::Mesh::Mesh* triangle = new LavaCake::Helpers::Mesh::Mesh();
	triangle->Data = {
		-0.75f, 0.75f , 0.0f,
		1.0f	, 0.0f	, 0.0f,
		 0.75f,	0.75f , 0.0f,
		 0.0f	, 1.0f	, 0.0f,
		 0.0f , -0.75f, 0.0f,
		 0.0f	, 0.0f	, 1.0f
	};
	triangle->index = { 0,1,2 };
	triangle->indexed = true;
	triangle->Parts = { {uint32_t(0),uint32_t(3)} };
	VertexBuffer* triangle_vertex_buffer = new VertexBuffer({ triangle }, {3,3});
	triangle_vertex_buffer->allocate(queue, commandBuffer[0].getHandle());

	RenderPass* pass = new RenderPass();
	GraphicPipeline* pipeline = new GraphicPipeline(LavaCake::vec3f({ 0,0,0 }), LavaCake::vec3f({ float(size.width),float(size.height),1.0f }), LavaCake::vec2f({ 0,0 }), LavaCake::vec2f({ float(size.width),float(size.height) }));
	VertexShaderModule* vertexShader = new VertexShaderModule("Data/Shaders/helloworld/shader.vert.spv");
	FragmentShaderModule* fragmentShader = new FragmentShaderModule("Data/Shaders/helloworld/shader.frag.spv");

	pipeline->setVextexShader(vertexShader);
	pipeline->setFragmentModule(fragmentShader);
	pipeline->setVeritices(triangle_vertex_buffer);

	pass->addSubPass({ pipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);
	pass->addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass->addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	
	pass->compile();

	std::vector<FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new FrameBuffer(s->size().width, s->size().height));
		pass->prepareOutputFrameBuffer(*frameBuffers[i]);
	}

	int f = 0;
	while (w.running()) {
		w.updateInput();
		f++;
		f = f % nbFrames;
		VkDevice logical = d->getLogicalDevice();
		VkSwapchainKHR& swapchain = s->getHandle();
		SwapChainImage& image = s->AcquireImage();

		std::vector<LavaCake::Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		commandBuffer[f].wait(2000000000);
		commandBuffer[f].resetFence();
		commandBuffer[f].beginRecord();

		
		

		pass->setSwapChainImage(*frameBuffers[f], image);

		pass->draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), LavaCake::vec2u({ 0,0 }), LavaCake::vec2u({ size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });

		commandBuffer[f].endRecord();

		
		if (!LavaCake::Command::SubmitCommandBuffersToQueue(queue, wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(0) }, commandBuffer[f].getFence())) {
			continue;
		}


		LavaCake::Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                              // uint32_t               ImageIndex
		};

		if (!LavaCake::Presentation::PresentImage(present_queue, { commandBuffer[f].getSemaphore(0) }, { present_info })) {
			continue;
		}
	}
	d->end();
}