#include "Framework/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"

using namespace LavaCake;

int main() {
	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Refraction", 500, 500);

	Framework::Mouse* mouse = Framework::Mouse::getMouse();
	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init();
	VkQueue queue = d->getGraphicQueue(0)->getHandle();
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	VkDevice logical = d->getLogicalDevice();
	VkExtent2D size = s->size();

	std::vector<Framework::CommandBuffer> commandBuffer = std::vector<Framework::CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
		commandBuffer[i].addSemaphore();
	}

	//cubeMap
	Framework::TextureBuffer* cubeMap = new Framework::CubeMap("Data/Textures/Skansen/", 4);
	cubeMap->allocate(queue, commandBuffer[0].getHandle());

	//cube vertices
	Helpers::Mesh::Mesh*  m = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/cube.obj", false, false, false, false, *m)) {
		return false;
	}
	Framework::VertexBuffer* v = new Framework::VertexBuffer({ m }, { 3 });
	v->allocate(queue, commandBuffer[0].getHandle());


	//teapotVertices
	Helpers::Mesh::Mesh*  teapot_mesh = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/teapot.obj", true, false, false, true, *teapot_mesh)) {
		return false;
	}
	Framework::VertexBuffer* teapot_vertex_buffer = new Framework::VertexBuffer({ teapot_mesh }, { 3,3 });
	teapot_vertex_buffer->allocate(queue, commandBuffer[0].getHandle());

	//uniform buffer
	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
		50.0f, 0.5f, 10.0f);

	mat4 modelView = Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	b->addVariable("modelView", modelView);
	b->addVariable("projection", proj);
	b->end();


	// Render pass
	Framework::RenderPass pass = Framework::RenderPass();

	// Skybox
	Framework::GraphicPipeline* skybox = new Framework::GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	Framework::VertexShaderModule* skyboxVertex = new Framework::VertexShaderModule("Data/Shaders/Refraction/skybox.vert.spv");
	skybox->setVextexShader(skyboxVertex);
	Framework::FragmentShaderModule* skyboxFrag = new Framework::FragmentShaderModule("Data/Shaders/Refraction/skybox.frag.spv");
	skybox->setFragmentModule(skyboxFrag);
	skybox->setVeritices(v);
	skybox->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);
	skybox->addTextureBuffer(cubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	skybox->SetCullMode(VK_CULL_MODE_FRONT_BIT);

	// teapot
	Framework::GraphicPipeline* teapot = new Framework::GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	Framework::VertexShaderModule* vertex = new Framework::VertexShaderModule("Data/Shaders/Refraction/model.vert.spv");
	teapot->setVextexShader(vertex);
	Framework::FragmentShaderModule* frag = new Framework::FragmentShaderModule("Data/Shaders/Refraction/model.frag.spv");
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


	std::vector<Framework::FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new Framework::FrameBuffer(s->size().width, s->size().height));
		pass.prepareOutputFrameBuffer(*frameBuffers[i]);
	}

	bool updateUniformBuffer = true;
	int f = 0;

	vec2d* lastMousePos = nullptr;

	vec2d polars = vec2d({ 0.0,0.0 });
	while (w.running()) {
		w.updateInput();
		f++;
		f = f % nbFrames;

		if (mouse->leftButton) {
			if (lastMousePos == nullptr) {
				lastMousePos = new vec2d({ mouse->position[0], mouse->position[1] });
			}
			polars[0] += float((mouse->position[0] - (*lastMousePos)[0]) / 512.0f) * 360;
			polars[1] -= float((mouse->position[1] - (*lastMousePos)[1]) / 512.0f) * 360;

			updateUniformBuffer = true;
			modelView = Helpers::Identity();

			modelView = modelView * Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);

			modelView = modelView * Helpers::PrepareRotationMatrix(-float(polars[0]), vec3f({ 0 , 1, 0 }));
			modelView = modelView * Helpers::PrepareRotationMatrix(float(polars[1]), vec3f({ 1 , 0, 0 }));
			//std::cout << w.m_mouse.position[0] << std::endl;
			b->setVariable("modelView", modelView);
			lastMousePos = new vec2d({ mouse->position[0], mouse->position[1] });
		}
		else {
			lastMousePos = nullptr;
		}


		

		Framework::SwapChainImage& image = s->AcquireImage();

		std::vector<LavaCake::Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});


		commandBuffer[f].wait(2000000000);
		commandBuffer[f].resetFence();
		commandBuffer[f].beginRecord();


		if (updateUniformBuffer) {
			b->update(commandBuffer[f].getHandle());
			updateUniformBuffer = false;
		}

		pass.setSwapChainImage(*frameBuffers[f], image);


		pass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), vec2u({ 0,0 }), vec2u({ size.width, size.height }) , { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });

		commandBuffer[f].endRecord();

		
		if (!Command::SubmitCommandBuffersToQueue(queue, wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(1) }, commandBuffer[f].getFence())) {
			continue;
		}

		Presentation::PresentInfo present_info = {
			s->getHandle(),                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                                   // uint32_t               ImageIndex
		};
		if (!Presentation::PresentImage(present_queue, { commandBuffer[f].getSemaphore(1) }, { present_info })) {
			continue;
		}
	}

	d->end();
}

