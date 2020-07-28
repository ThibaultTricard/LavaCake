#include "LavaCake/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"

using namespace LavaCake;

int main() {
	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Refraction", 0, 0, 500, 500);

	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	d->prepareFrames(nbFrames);


	//cubeMap
	Framework::TextureBuffer* cubeMap = new Framework::CubeMap("Data/Textures/Skansen/", 4);
	cubeMap->allocate();

	//cube vertices
	Helpers::Mesh::Mesh*  m = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/cube.obj", false, false, false, false, *m)) {
		return false;
	}
	Framework::VertexBuffer* v = new Framework::VertexBuffer({ m }, { 3 });
	v->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().commandBuffer);


	//teapotVertices
	Helpers::Mesh::Mesh*  teapot_mesh = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/teapot.obj", true, false, false, true, *teapot_mesh)) {
		return false;
	}
	Framework::VertexBuffer* teapot_vertex_buffer = new Framework::VertexBuffer({ teapot_mesh }, { 3,3 });
	teapot_vertex_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().commandBuffer);

	//uniform buffer
	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::Transformation::PreparePerspectiveProjectionMatrix(static_cast<float>(w.m_windowSize[0]) / static_cast<float>(w.m_windowSize[1]),
		50.0f, 0.5f, 10.0f);

	mat4 modelView = Helpers::Transformation::PrepareTranslationMatrix(0.0f, 0.0f, 4.0f);
	b->addVariable("modelView", modelView);
	b->addVariable("projection", proj);
	b->end();


	// Render pass
	Framework::RenderPass pass = Framework::RenderPass();

	// Skybox
	Framework::GraphicPipeline* skybox = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });
	Framework::VertexShaderModule* skyboxVertex = new Framework::VertexShaderModule("Data/Shaders/11 Lighting/04 Rendering a reflective and refractive geometry using cubemaps/skybox.vert.spv");
	skybox->setVextexShader(skyboxVertex);
	Framework::FragmentShaderModule* skyboxFrag = new Framework::FragmentShaderModule("Data/Shaders/11 Lighting/04 Rendering a reflective and refractive geometry using cubemaps/skybox.frag.spv");
	skybox->setFragmentModule(skyboxFrag);
	skybox->setVeritices(v);
	skybox->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);
	skybox->addTextureBuffer(cubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	skybox->SetCullMode(VK_CULL_MODE_FRONT_BIT);

	// teapot
	Framework::GraphicPipeline* teapot = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });
	Framework::VertexShaderModule* vertex = new Framework::VertexShaderModule("Data/Shaders/11 Lighting/04 Rendering a reflective and refractive geometry using cubemaps/model.vert.spv");
	teapot->setVextexShader(vertex);
	Framework::FragmentShaderModule* frag = new Framework::FragmentShaderModule("Data/Shaders/11 Lighting/04 Rendering a reflective and refractive geometry using cubemaps/model.frag.spv");
	teapot->setFragmentModule(frag);
	teapot->setVeritices(teapot_vertex_buffer);
	teapot->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);
	teapot->addTextureBuffer(cubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	teapot->SetCullMode(VK_CULL_MODE_BACK_BIT);

	Framework::PushConstant* constant = new Framework::PushConstant();
	vec3f camera = vec3f({ 0.f,0.f,4.f });
	constant->addVariable("camera", camera);
	teapot->addPushContant(constant, VK_SHADER_STAGE_FRAGMENT_BIT);

	pass.addSubPass({ skybox, teapot }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);
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

			modelView = modelView * Helpers::Transformation::PrepareTranslationMatrix(0.0f, 0.0f,  w.m_mouse.m_wheel.distance- 4.0f);

			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(-float(w.m_mouse.m_position.x) / float(w.m_windowSize[0]) * 360 - 180, { 0 , 1, 0 });
			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(float(w.m_mouse.m_position.y) / float(w.m_windowSize[1]) * 360 - 180, { 1 , 0, 0 });

			camera = vec3f({ 0, 0, 4 }) * modelView;

		  b->setVariable("modelView", modelView);
			constant->setVariable("camera", camera);
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


		pass.draw(frame.commandBuffer, *frame.framebuffer, { 0,0 }, { int(size.width), int(size.height) } , { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });



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

