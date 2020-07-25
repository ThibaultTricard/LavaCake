#include "LavaCake/Framework.h"

using namespace LavaCake::Framework;

int main() {
	Window w("HelloWolrd", 0, 0, 512, 512);
	w.Show();

	int nbFrames = 3;

	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	d->prepareFrames(nbFrames);


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
	triangle_vertex_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().CommandBuffer);

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



		LavaCake::Buffer::FrameResources& frame = d->getFrameRessources()->at(f);
		VkCommandBuffer commandbuffer = frame.CommandBuffer;
		VkDevice logical = d->getLogicalDevice();
		VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();
		VkQueue& present_queue = d->getPresentQueue()->getHandle();

		if (!LavaCake::Fence::WaitForFences(logical, { *frame.DrawingFinishedFence }, false, 2000000000)) {
			continue;
		}
		if (!LavaCake::Fence::ResetFences(logical, { *frame.DrawingFinishedFence })) {
			continue;
		}

		InitVkDestroyer(logical, frame.Framebuffer);
		if (!LavaCake::Command::BeginCommandBufferRecordingOperation(commandbuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
			continue;
		}



		VkSwapchainKHR& swapchain = d->getSwapChain().getHandle();
		std::vector<VkImageView>& swapchain_image_views = d->getSwapChain().getImageView();
		VkSemaphore  image_acquired_semaphore = *frame.ImageAcquiredSemaphore;
		VkImageView depth_attachment = *frame.DepthAttachment;
		uint32_t image_index;
		VkExtent2D size = d->getSwapChain().size();

		if (!LavaCake::Swapchain::AcquireSwapchainImage(logical, swapchain, image_acquired_semaphore, VK_NULL_HANDLE, image_index)) {
			continue;
		}

		std::vector<VkImageView> attachments = { swapchain_image_views[image_index] };
		if (VK_NULL_HANDLE != depth_attachment) {
			attachments.push_back(depth_attachment);
		}
		if (!LavaCake::Buffer::CreateFramebuffer(logical, pass->getHandle(), attachments, size.width, size.height, 1, *frame.Framebuffer)) {
			continue;
		}


		pass->draw(frame.CommandBuffer, *frame.Framebuffer, { 0,0 }, { int(size.width), int(size.height) }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });



		if (!LavaCake::Command::EndCommandBufferRecordingOperation(commandbuffer)) {
			continue;
		}


		std::vector<LavaCake::Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image_acquired_semaphore,                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT // VkPipelineStageFlags   WaitingStage
			});
		if (!LavaCake::Command::SubmitCommandBuffersToQueue(graphics_queue, wait_semaphore_infos, { commandbuffer }, { *frame.ReadyToPresentSemaphore }, *frame.DrawingFinishedFence)) {
			continue;
		}

		LavaCake::Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image_index                                   // uint32_t               ImageIndex
		};
		if (!LavaCake::Presentation::PresentImage(present_queue, { *frame.ReadyToPresentSemaphore }, { present_info })) {
			continue;
		}

	}


	d->end();
}