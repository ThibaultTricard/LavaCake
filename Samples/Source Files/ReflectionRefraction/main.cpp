#include "Framework/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"
#include "Geometry/meshLoader.h"
#include "Geometry/meshExporter.h"

using namespace LavaCake;
using namespace LavaCake::Framework;
using namespace LavaCake::Geometry;
using namespace LavaCake::Core;

int main() {
	int nbFrames = 3;
	ErrorCheck::PrintError(true);
	Window w("LavaCake : Refraction", 500, 500);

	Mouse* mouse = Mouse::getMouse();
	Device* d = Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	SwapChain* s = SwapChain::getSwapChain();
	s->init();
	Queue* queue = d->getGraphicQueue(0);
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	VkDevice logical = d->getLogicalDevice();
	VkExtent2D size = s->size();

	std::vector<CommandBuffer> commandBuffer = std::vector<CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
		commandBuffer[i].addSemaphore();
	}

	//cubeMap
	TextureBuffer* cubeMap = new CubeMap("Data/Textures/Skansen/", 4);
	cubeMap->allocate(queue, commandBuffer[0]);

	//Skybox Data

	std::pair<std::vector<float>, vertexFormat > sky = Load3DModelFromObjFile("Data/Models/cube.obj", true, false, false, true);
	Mesh_t* sky_mesh = new Mesh<TRIANGLE>(sky.first, sky.second);

	VertexBuffer* v = new VertexBuffer({ sky_mesh });
	v->allocate(queue, commandBuffer[0]);


	//teapotVertices

	std::pair<std::vector<float>, vertexFormat > teapot = Load3DModelFromObjFile("Data/Models/teapot.obj", true, false, false, true);
	Mesh_t* teapot_mesh = new Mesh<TRIANGLE>(teapot.first, teapot.second);

	exportToPly(teapot_mesh, "teapot.ply");

	VertexBuffer* teapot_vertex_buffer = new VertexBuffer({ teapot_mesh });
	teapot_vertex_buffer->allocate(queue, commandBuffer[0]);

	//uniform buffer
	UniformBuffer* b = new UniformBuffer();
	mat4 proj = LavaCake::Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
		50.0f, 0.5f, 10.0f);

	mat4 modelView = Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	b->addVariable("modelView", &modelView);
	b->addVariable("projection", &proj);
	b->end();


	// Render pass
	RenderPass pass = RenderPass();

	// Skybox
	GraphicPipeline* skybox = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	VertexShaderModule* skyboxVertex = new VertexShaderModule("Data/Shaders/Refraction/skybox.vert.spv");
	skybox->setVextexShader(skyboxVertex);
	FragmentShaderModule* skyboxFrag = new FragmentShaderModule("Data/Shaders/Refraction/skybox.frag.spv");
	skybox->setFragmentModule(skyboxFrag);
	skybox->setVertices(v);
	skybox->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);
	skybox->addTextureBuffer(cubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	skybox->SetCullMode(VK_CULL_MODE_FRONT_BIT);

	// teapot
	GraphicPipeline* teapotPipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	VertexShaderModule* vertex = new VertexShaderModule("Data/Shaders/Refraction/model.vert.spv");
	teapotPipeline->setVextexShader(vertex);
	FragmentShaderModule* frag = new FragmentShaderModule("Data/Shaders/Refraction/model.frag.spv");
	teapotPipeline->setFragmentModule(frag);
	teapotPipeline->setVertices(teapot_vertex_buffer);
	teapotPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);
	teapotPipeline->addTextureBuffer(cubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	teapotPipeline->SetCullMode(VK_CULL_MODE_BACK_BIT);

	PushConstant* constant = new PushConstant();
	vec3f camera = vec3f({ 0.f,0.f,4.f });
	constant->addVariable("camera", &camera);
	teapotPipeline->addPushContant(constant, VK_SHADER_STAGE_FRAGMENT_BIT);

	pass.addSubPass({ skybox, teapotPipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);
	pass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass.compile();


	std::vector<FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new FrameBuffer(s->size().width, s->size().height));
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
			b->setVariable("modelView", &modelView);
			lastMousePos = new vec2d({ mouse->position[0], mouse->position[1] });
		}
		else {
			lastMousePos = nullptr;
		}


		

		SwapChainImage& image = s->AcquireImage();

		std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
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

		
		if (!SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(1) }, commandBuffer[f].getFence())) {
			continue;
		}

		PresentInfo present_info = {
			s->getHandle(),                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                                   // uint32_t               ImageIndex
		};
		if (!PresentImage(present_queue, { commandBuffer[f].getSemaphore(1) }, { present_info })) {
			continue;
		}
	}

	d->end();
}

