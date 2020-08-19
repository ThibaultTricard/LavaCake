#include "LavaCake/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"

using namespace LavaCake;

int main() {
	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Bump Mapping", 0, 0, 500, 500);

	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	d->prepareFrames(nbFrames);


	//Normal map
	Framework::TextureBuffer* normalMap = new Framework::TextureBuffer("Data/Textures/normal_map.png", 4);
	normalMap->allocate();

	//vertex buffer
	Helpers::Mesh::Mesh*  m = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/ice.obj", true, true, true, true, *m)) {
		return false;
	}
	Framework::VertexBuffer* v = new Framework::VertexBuffer({ m }, { 3,3,2,3,3 });
	v->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().commandBuffer);

	//uniform buffer
	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::Transformation::PreparePerspectiveProjectionMatrix(static_cast<float>(w.m_windowSize[0]) / static_cast<float>(w.m_windowSize[1]),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = Helpers::Transformation::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	b->addVariable("modelView", modelView);
	b->addVariable("projection", proj);
	b->end();


	//PushConstant
	Framework::PushConstant* constant = new Framework::PushConstant();
	vec4f LigthPos = vec4f({ 0.f,4.f,0.7f,0.0 });
	constant->addVariable("LigthPos", LigthPos);

	// Render pass
	Framework::RenderPass pass = Framework::RenderPass();
	Framework::GraphicPipeline* pipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });

	Framework::VertexShaderModule* vertex = new Framework::VertexShaderModule("Data/Shaders/NormalMap/shader.vert.spv");
	pipeline->setVextexShader(vertex);


	Framework::FragmentShaderModule* frag = new Framework::FragmentShaderModule("Data/Shaders/NormalMap/shader.frag.spv");
	pipeline->setFragmentModule(frag);

	pipeline->setVeritices(v);
	pipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);
	pipeline->addTextureBuffer(normalMap, VK_SHADER_STAGE_FRAGMENT_BIT,1);
	pipeline->addPushContant(constant, VK_SHADER_STAGE_FRAGMENT_BIT);

	pass.addSubPass({ pipeline }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);
	pass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass.compile();


	w.Show();
	bool updateUniformBuffer = true;
	int f = 0;
	while (w.m_loop) {
		w.UpdateInput();
		f++;
		f = f % nbFrames;

		if (w.m_mouse.m_actionPerformed) {
			updateUniformBuffer = true;
			modelView = Helpers::Transformation::Identity();

			modelView = modelView * Helpers::Transformation::PrepareTranslationMatrix(0.0f, 0.0f, w.m_mouse.m_wheel.distance - 4.0f);

			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(-float(w.m_mouse.m_position.x) / float(w.m_windowSize[0]) * 360, { 0 , 1, 0 });
			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(float(w.m_mouse.m_position.y) / float(w.m_windowSize[1]) * 360, { 1 , 0, 0 });

			b->setVariable("modelView", modelView);
		}



		Buffer::FrameResources& frame = d->getFrameRessources()->at(f);
		VkCommandBuffer commandbuffer = frame.commandBuffer;
		VkDevice logical = d->getLogicalDevice();
		VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();
		VkQueue& present_queue = d->getPresentQueue()->getHandle();

		if (!Fence::WaitForFences(logical, { *frame.drawingFinishedFence }, false, 2000000000)) {
			continue;
		}
		if (!Fence::ResetFences(logical, { *frame.drawingFinishedFence })) {
			continue;
		}

		InitVkDestroyer(logical, frame.framebuffer);
		if (!Command::BeginCommandBufferRecordingOperation(commandbuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
			continue;
		}


		if (updateUniformBuffer) {
			b->update(commandbuffer);
			updateUniformBuffer = false;
		}



		VkSwapchainKHR& swapchain = d->getSwapChain().getHandle();
		std::vector<VkImageView>& swapchain_image_views = d->getSwapChain().getImageView();
		VkSemaphore  image_acquired_semaphore = *frame.imageAcquiredSemaphore;
		VkImageView depth_attachment = *frame.depthAttachment;
		uint32_t image_index;
		VkExtent2D size = d->getSwapChain().size();

		if (!Swapchain::AcquireSwapchainImage(logical, swapchain, image_acquired_semaphore, VK_NULL_HANDLE, image_index)) {
			continue;
		}

		std::vector<VkImageView> attachments = { swapchain_image_views[image_index] };
		if (VK_NULL_HANDLE != depth_attachment) {
			attachments.push_back(depth_attachment);
		}
		if (!Buffer::CreateFramebuffer(logical, pass.getHandle(), attachments, size.width, size.height, 1, *frame.framebuffer)) {
			continue;
		}


		pass.draw(frame.commandBuffer, *frame.framebuffer, { 0,0 }, { size.width, size.height }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });



		if (!LavaCake::Command::EndCommandBufferRecordingOperation(commandbuffer)) {
			continue;
		}


		std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image_acquired_semaphore,                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT // VkPipelineStageFlags   WaitingStage
			});
		if (!Command::SubmitCommandBuffersToQueue(graphics_queue, wait_semaphore_infos, { commandbuffer }, { *frame.readyToPresentSemaphore }, *frame.drawingFinishedFence)) {
			continue;
		}

		Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image_index                                   // uint32_t               ImageIndex
		};
		if (!Presentation::PresentImage(present_queue, { *frame.readyToPresentSemaphore }, { present_info })) {
			continue;
		}
	}
	d->end();
}

