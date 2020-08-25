#include "LavaCake/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"

using namespace LavaCake;

int main() {
	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Specular Lighting", 0, 0, 512, 512);

	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init(nbFrames);
	VkQueue queue = d->getGraphicQueue(0)->getHandle();
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	std::vector<Framework::CommandBuffer> commandBuffer = std::vector<Framework::CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
		commandBuffer[i].addSemaphore();
	}

	//vertex buffer
	Helpers::Mesh::Mesh*  m = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/knot.obj", true, false, false, true, *m)) {
		return false;
	}
	Framework::VertexBuffer* v = new Framework::VertexBuffer({ m }, { 3,3 });
	v->allocate(queue, commandBuffer[0].getHandle());

	//uniform buffer
	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(w.m_windowSize[0]) / static_cast<float>(w.m_windowSize[1]),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	b->addVariable("modelView", modelView);
	b->addVariable("projection", proj);
	b->end();


	// Render pass
	Framework::RenderPass pass = Framework::RenderPass();
	Framework::GraphicPipeline* pipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });

	Framework::VertexShaderModule* vertex = new Framework::VertexShaderModule("Data/Shaders/SpecularLighting/shader.vert.spv");
	pipeline->setVextexShader(vertex);

	Framework::FragmentShaderModule* frag = new Framework::FragmentShaderModule("Data/Shaders/SpecularLighting/shader.frag.spv");
	pipeline->setFragmentModule(frag);

	pipeline->setVeritices(v);
	pipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);

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
			modelView = Helpers::Identity();

			modelView = modelView * Helpers::PrepareTranslationMatrix(0.0f, 0.0f, w.m_mouse.m_wheel.distance - 4.0f);

			modelView = modelView * Helpers::PrepareRotationMatrix(-float(w.m_mouse.m_position.x) / float(w.m_windowSize[0]) * 360, { 0 , 1, 0 });
			modelView = modelView * Helpers::PrepareRotationMatrix(float(w.m_mouse.m_position.y) / float(w.m_windowSize[1]) * 360, { 1 , 0, 0 });

			b->setVariable("modelView", modelView);
		}



		Buffer::FrameResources& frame = s->getFrameRessources()->at(f);
		VkDevice logical = d->getLogicalDevice();
		VkQueue& present_queue = d->getPresentQueue()->getHandle();
		VkSwapchainKHR& swapchain = s->getHandle();
		std::vector<VkImageView>& swapchain_image_views = s->getImageView();
		VkImageView depth_attachment = *frame.depthAttachment;
		uint32_t image_index;
		VkExtent2D size = s->size();

		if (!Swapchain::AcquireSwapchainImage(logical, swapchain, commandBuffer[f].getSemaphore(0), VK_NULL_HANDLE, image_index)) {
			continue;
		}
		std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			commandBuffer[f].getSemaphore(0),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		commandBuffer[f].wait(2000000000);
		commandBuffer[f].resetFence();
		commandBuffer[f].beginRecord();


		if (updateUniformBuffer) {
			b->update(commandBuffer[f].getHandle());
			updateUniformBuffer = false;
		}

		

		std::vector<VkImageView> attachments = { swapchain_image_views[image_index] };
		if (VK_NULL_HANDLE != depth_attachment) {
			attachments.push_back(depth_attachment);
		}
		if (!Buffer::CreateFramebuffer(logical, pass.getHandle(), attachments, size.width, size.height, 1, *frame.framebuffer)) {
			continue;
		}


		pass.draw(commandBuffer[f].getHandle(), *frame.framebuffer, { 0,0 }, { size.width, size.height }, {{ 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 }});



		commandBuffer[f].endRecord();


		
		if (!Command::SubmitCommandBuffersToQueue(queue, wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(1) }, commandBuffer[f].getFence())) {
			continue;
		}

		Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image_index                                   // uint32_t               ImageIndex
		};
		if (!Presentation::PresentImage(present_queue, { commandBuffer[f].getSemaphore(1) }, { present_info })) {
			continue;
		}
	}

	d->end();
}
