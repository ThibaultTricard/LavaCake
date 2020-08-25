#include "LavaCake/Framework.h"
using namespace LavaCake::Framework;
int main() {
	Window w("LavaCake HelloWorld", 0, 0, 512, 512);
	w.Show();

	int nbFrames = 3;
	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init(nbFrames); 
	VkQueue queue = d->getGraphicQueue(0)->getHandle();
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	std::vector<CommandBuffer> commandBuffer = std::vector<CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
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
	triangle->Parts = { {uint32_t(0),uint32_t(3)} };
	VertexBuffer* triangle_vertex_buffer = new VertexBuffer({ triangle }, {3,3});
	triangle_vertex_buffer->allocate(queue, commandBuffer[0].getHandle());


	RenderPass* pass = new RenderPass();

	GraphicPipeline* pipeline = new GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });
	VertexShaderModule* vertexShader = new VertexShaderModule("Data/Shaders/helloworld/shader.vert.spv");
	FragmentShaderModule* fragmentShader = new FragmentShaderModule("Data/Shaders/helloworld/shader.frag.spv");
	pipeline->setVextexShader(vertexShader);
	pipeline->setFragmentModule(fragmentShader);
	pipeline->setVeritices(triangle_vertex_buffer);
	
	pass->addSubPass({ pipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);
	pass->addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass->addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass->compile();

	int f = 0;
	while (w.m_loop) {
		w.UpdateInput();
		f++;
		f = f % nbFrames;
		LavaCake::Buffer::FrameResources& frame = s->getFrameRessources()->at(f);
		VkDevice logical = d->getLogicalDevice();
		VkSwapchainKHR& swapchain = s->getHandle();
		std::vector<VkImageView>& swapchain_image_views = s->getImageView();
		VkImageView depth_attachment = *frame.depthAttachment;
		uint32_t image_index;
		VkExtent2D size = s->size();

		commandBuffer[f].wait(2000000000);
		commandBuffer[f].resetFence();
		commandBuffer[f].beginRecord();

		

		if (!LavaCake::Swapchain::AcquireSwapchainImage(logical, swapchain, commandBuffer[f].getSemaphore(0), VK_NULL_HANDLE, image_index)) {
			continue;
		}

		std::vector<VkImageView> attachments = { swapchain_image_views[image_index] };

		if (VK_NULL_HANDLE != depth_attachment) {
			attachments.push_back(depth_attachment);
		}


		InitVkDestroyer(logical, frame.framebuffer);
		if (!LavaCake::Buffer::CreateFramebuffer(logical, pass->getHandle(), attachments, size.width, size.height, 1, *frame.framebuffer)) {
			continue;
		}

		pass->draw(commandBuffer[f].getHandle(), *frame.framebuffer, { 0,0 }, { size.width, size.height }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });

		commandBuffer[f].endRecord();

		std::vector<LavaCake::Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			commandBuffer[f].getSemaphore(0),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		if (!LavaCake::Command::SubmitCommandBuffersToQueue(queue, wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(1) }, commandBuffer[f].getFence())) {
			continue;
		}
		LavaCake::Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image_index                                   // uint32_t               ImageIndex
		};

		if (!LavaCake::Presentation::PresentImage(present_queue, { commandBuffer[f].getSemaphore(1) }, { present_info })) {
			continue;
		}
	}
	d->end();
}