#include "Framework/Framework.h"

using namespace LavaCake;

int main() {
	uint32_t nbFrames = 4;
	Framework::ErrorCheck::PrintError(true);

	Framework::Window w("LavaCake HelloWorld", 512, 512);

	Framework::Mouse* mouse = Framework::Mouse::getMouse();

	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init();

	VkQueue queue = d->getGraphicQueue(0)->getHandle();
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	std::vector<Framework::CommandBuffer> commandBuffer = std::vector<Framework::CommandBuffer>(nbFrames);
	VkExtent2D size = s->size();

	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
	}

	vec3f camera = vec3f({0.0f,0.0f,4.0f});

	//knot mesh
	Helpers::Mesh::Mesh*  knot_mesh = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/knot.obj", true, false, false, true, *knot_mesh)) {
		return false;
	}
	//plane mesh
	Helpers::Mesh::Mesh*  plane_mesh = new Helpers::Mesh::Mesh();
	if (!Helpers::Mesh::Load3DModelFromObjFile("Data/Models/plane.obj", true, false, false, false, *plane_mesh)) {
		return false;
	}

	Framework::VertexBuffer* scene_vertex_buffer = new Framework::VertexBuffer({ plane_mesh, knot_mesh  }, { 3,3 });
	scene_vertex_buffer->allocate(queue, commandBuffer[0].getHandle());

	Framework::VertexBuffer* plane_buffer = new Framework::VertexBuffer({ plane_mesh }, { 3,3 });
	plane_buffer->allocate(queue, commandBuffer[0].getHandle());


	//PostProcessQuad
	Helpers::Mesh::Mesh* quad = new Helpers::Mesh::Mesh();
	Helpers::Mesh::preparePostProcessQuad(*quad, true);
	Framework::VertexBuffer* quad_vertex_buffer = new Framework::VertexBuffer({ quad }, { 3,2 });
	quad_vertex_buffer->allocate(queue, commandBuffer[0].getHandle());

	//uniform buffer
	uint32_t shadowsize = 64;

	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(shadowsize) / static_cast<float>(shadowsize),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = mat4 { 0.9981f, -0.0450f, 0.0412f, 0.0000f, 0.0000f, 0.6756f, 0.7373f, 0.0000f, -0.0610f, -0.7359f, 0.6743f, 0.0000f, -0.0000f, -0.0000f, -4.0000f, 1.0000f };
	mat4 lightView = mat4{ 1.0f,0.0f,0.0f,0.0f,0.0f,0.173648223f ,0.984807730f,0.0f,0.0f, -0.984807730f, 0.173648223f ,0.0f,0.0f,0.0f,-3.99999976f ,1.0f };
	b->addVariable("ligthView", lightView);
	b->addVariable("modelView", modelView);
	b->addVariable("projection", proj);
	b->end();

	//PushConstant
	Framework::PushConstant* constant = new Framework::PushConstant();
	vec4f LigthPos = vec4f({ 0.f,4.f,0.7f,0.0 });
	constant->addVariable("LigthPos", LigthPos);


	//DepthBuffer
	Framework::FrameBuffer* shadow_map_buffer = new Framework::FrameBuffer(shadowsize, shadowsize);

	//frameBuffer
	Framework::FrameBuffer* scene_buffer = new Framework::FrameBuffer(shadowsize, shadowsize);
	
	// Shadow pass
	Framework::RenderPass shadowMapPass = Framework::RenderPass();
	Framework::GraphicPipeline* shadowPipeline = new Framework::GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(shadowsize),float(shadowsize),1.0f }), vec2f({ 0,0 }), vec2f({ float(shadowsize),float(shadowsize) }));

	Framework::VertexShaderModule* shadowVertex = new Framework::VertexShaderModule("Data/Shaders/ConsoleRenderer/shadow.vert.spv");
	shadowPipeline->setVextexShader(shadowVertex);


	shadowPipeline->setVeritices(scene_vertex_buffer);
	shadowPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);


	shadowMapPass.addSubPass({ shadowPipeline }, Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_DEPTH);
	shadowMapPass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	shadowMapPass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	shadowMapPass.compile();

	shadowMapPass.prepareOutputFrameBuffer(*shadow_map_buffer);

	//Render Pass
	Framework::RenderPass renderPass = Framework::RenderPass();
	Framework::GraphicPipeline* renderPipeline = new Framework::GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(shadowsize),float(shadowsize),1.0f }), vec2f({ 0,0 }), vec2f({ float(shadowsize),float(shadowsize) }));
	Framework::VertexShaderModule* renderVertex = new Framework::VertexShaderModule("Data/Shaders/ConsoleRenderer/scene.vert.spv");
	renderPipeline->setVextexShader(renderVertex);

	Framework::FragmentShaderModule* renderFrag = new Framework::FragmentShaderModule("Data/Shaders/ConsoleRenderer/scene.frag.spv");
	renderPipeline->setFragmentModule(renderFrag);

	
	renderPipeline->addPushContant(constant, VK_SHADER_STAGE_VERTEX_BIT);

	renderPipeline->setVeritices(scene_vertex_buffer);
	renderPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);

	renderPipeline->addFrameBuffer(shadow_map_buffer, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	renderPass.addSubPass({ renderPipeline },  Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);
	renderPass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.compile();

	renderPass.prepareOutputFrameBuffer(*scene_buffer);

	//Console Render pass
	Framework::RenderPass consolePass = Framework::RenderPass();
	Framework::GraphicPipeline* consolePipeline = new Framework::GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	Framework::VertexShaderModule* consoleVertex = new Framework::VertexShaderModule("Data/Shaders/ConsoleRenderer/console.vert.spv");
	consolePipeline->setVextexShader(consoleVertex);

	Framework::FragmentShaderModule* consoleFrag = new Framework::FragmentShaderModule("Data/Shaders/ConsoleRenderer/console.frag.spv");
	consolePipeline->setFragmentModule(consoleFrag);


	consolePipeline->setVeritices(quad_vertex_buffer);

	consolePipeline->addFrameBuffer(scene_buffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	consolePass.addSubPass({ consolePipeline }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);
	consolePass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	consolePass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	consolePass.compile();


	std::vector<Framework::FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new Framework::FrameBuffer(s->size().width, s->size().height));
		consolePass.prepareOutputFrameBuffer(*frameBuffers[i]);
	}


	bool updateUniformBuffer = true;
	uint32_t f = 0;

	vec2d* lastMousePos = nullptr;

	vec2d polars = vec2d({ 0.0,0.0 });
	while (w.running()){
		w.updateInput();
		f++;
		f = f % nbFrames;
		
		VkDevice logical = d->getLogicalDevice();
		VkSwapchainKHR& swapchain = s->getHandle();
		
		Framework::SwapChainImage& image = s->AcquireImage();

		std::vector<LavaCake::Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		
		int state = glfwGetKey(w.m_window, GLFW_KEY_SPACE);
    if (state == GLFW_PRESS) {
			Command::WaitForAllSubmittedCommandsToBeFinished(logical);
			consolePass.reloadShaders();
		}


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

	

		commandBuffer[f].wait(2000000000);
		commandBuffer[f].resetFence();
		commandBuffer[f].beginRecord();


		if (updateUniformBuffer) {
			b->update(commandBuffer[f].getHandle());
			updateUniformBuffer = false;
		}



		shadowMapPass.draw(commandBuffer[f].getHandle(), shadow_map_buffer->getHandle(), vec2u({ 0,0 }), vec2u({shadowsize, shadowsize }));


		if (d->getPresentQueue()->getIndex() != d->getGraphicQueue(0)->getIndex()) {
			Image::ImageTransition image_transition_before_drawing = {
				image.getImage(),													// VkImage              Image
				VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        CurrentAccess
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // VkAccessFlags        NewAccess
				VK_IMAGE_LAYOUT_UNDEFINED,                // VkImageLayout        CurrentLayout
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout        NewLayout
				d->getPresentQueue()->getIndex(),         // uint32_t             CurrentQueueFamily
				d->getGraphicQueue(0)->getIndex(),        // uint32_t             NewQueueFamily
				VK_IMAGE_ASPECT_COLOR_BIT                 // VkImageAspectFlags   Aspect
			};
			Image::SetImageMemoryBarrier(commandBuffer[f].getHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
		}



		renderPass.draw(commandBuffer[f].getHandle(), scene_buffer->getHandle(), vec2u({ 0,0 }), scene_buffer->size(), { { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0 } });


		consolePass.setSwapChainImage(*frameBuffers[f], image);
		consolePass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), vec2u({ 0,0 }), vec2u({size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });


		if (d->getPresentQueue()->getIndex() != d->getGraphicQueue(0)->getIndex()) {
			Image::ImageTransition image_transition_before_drawing = {
				image.getImage(),														// VkImage              Image
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,				// VkAccessFlags        CurrentAccess
				VK_ACCESS_MEMORY_READ_BIT,									// VkAccessFlags        NewAccess
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,						// VkImageLayout        CurrentLayout
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,						// VkImageLayout        NewLayout
				d->getGraphicQueue(0)->getIndex(),          // uint32_t             CurrentQueueFamily
				d->getPresentQueue()->getIndex(),						// uint32_t             NewQueueFamily
				VK_IMAGE_ASPECT_COLOR_BIT										// VkImageAspectFlags   Aspect
			};
			Image::SetImageMemoryBarrier(commandBuffer[f].getHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
		}
		


		commandBuffer[f].endRecord();

		
		if (!Command::SubmitCommandBuffersToQueue(queue, wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(0) }, commandBuffer[f].getFence() )) {
			continue;
		}

		

		Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                              // uint32_t               ImageIndex
		};
		if (!Presentation::PresentImage(present_queue, { commandBuffer[f].getSemaphore(0) }, { present_info })) {
			continue;
		}

	}
	d->end();
}

