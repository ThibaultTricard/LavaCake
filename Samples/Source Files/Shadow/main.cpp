#include "Framework/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"
#include "Geometry/meshLoader.h"

using namespace LavaCake;

int main() {
	uint32_t nbFrames = 4;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Shadow", 1000, 800);
	Framework::Mouse* mouse = Framework::Mouse::getMouse();
	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init();
	VkDevice logical = d->getLogicalDevice();
	Framework::Queue* queue = d->getGraphicQueue(0);
	VkQueue& present_queue = d->getPresentQueue()->getHandle();

	VkExtent2D size = s->size();

	std::vector<Framework::CommandBuffer> commandBuffer = std::vector<Framework::CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
		commandBuffer[i].addSemaphore();
	}

	vec3f camera = vec3f({0.0f,0.0f,4.0f});

	//knot mesh
	std::pair<std::vector<float>, Geometry::vertexFormat > knot = Geometry::Load3DModelFromObjFile("Data/Models/knot.obj", true, false, false, true);
	Geometry::Mesh_t* knot_mesh = new Geometry::Mesh<Geometry::TRIANGLE>(knot.first, knot.second);


	//plane mesh
	std::pair<std::vector<float>, Geometry::vertexFormat > plane = Geometry::Load3DModelFromObjFile("Data/Models/plane.obj", true, false, false, false);
	Geometry::Mesh_t* plane_mesh = new Geometry::Mesh<Geometry::TRIANGLE>(plane.first, plane.second);



	Framework::VertexBuffer* scene_vertex_buffer = new Framework::VertexBuffer({ plane_mesh, knot_mesh });
	scene_vertex_buffer->allocate(queue, commandBuffer[0]);

	Framework::VertexBuffer* plane_buffer = new Framework::VertexBuffer({ plane_mesh });
	plane_buffer->allocate(queue, commandBuffer[0]);


	//uniform buffer
	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
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

	uint32_t shadowsize = 512;
	
	//frameBuffer
	Framework::FrameBuffer* shadow_map_buffer = new Framework::FrameBuffer(shadowsize, shadowsize);
	
	// Shadow pass
	Framework::RenderPass shadowMapPass = Framework::RenderPass();
	Framework::GraphicPipeline* shadowPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(shadowsize),float(shadowsize),1.0f }, { 0,0 }, { float(shadowsize),float(shadowsize) });

	Framework::VertexShaderModule* shadowVertex = new Framework::VertexShaderModule("Data/Shaders/Shadow/shadow.vert.spv");
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
	Framework::GraphicPipeline* renderPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(size.width),float(size.height),1.0f }, { 0,0 }, { float(size.width),float(size.height) });
	Framework::VertexShaderModule* renderVertex = new Framework::VertexShaderModule("Data/Shaders/Shadow/scene.vert.spv");
	renderPipeline->setVextexShader(renderVertex);

	Framework::FragmentShaderModule* renderFrag = new Framework::FragmentShaderModule("Data/Shaders/Shadow/scene.frag.spv");
	renderPipeline->setFragmentModule(renderFrag);

	
	renderPipeline->addPushContant(constant, VK_SHADER_STAGE_VERTEX_BIT);

	renderPipeline->setVeritices(scene_vertex_buffer);
	renderPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);

	renderPipeline->addFrameBuffer(shadow_map_buffer, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	renderPass.addSubPass({ renderPipeline }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);
	renderPass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.compile();


	std::vector<Framework::FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new Framework::FrameBuffer(s->size().width, s->size().height));
		renderPass.prepareOutputFrameBuffer(*frameBuffers[i]);
	}

	bool updateUniformBuffer = true;
	uint32_t f = 0;

	vec2d* lastMousePos = nullptr;

	vec2d polars = { 0.0,0.0 };
	while (w.running()) {

		
		
		VkSwapchainKHR& swapchain = s->getHandle();
		VkExtent2D size = s->size();
		Framework::SwapChainImage& image = s->AcquireImage();

		std::vector<LavaCake::Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});


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

			modelView = modelView * Helpers::PrepareRotationMatrix(-float(polars[0]), { 0 , 1, 0 });
			modelView = modelView * Helpers::PrepareRotationMatrix(float(polars[1]), { 1 , 0, 0 });
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

		shadowMapPass.draw(commandBuffer[f].getHandle(), shadow_map_buffer->getHandle(), { 0,0 }, { shadowsize, shadowsize });


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


		

	
		renderPass.setSwapChainImage(*frameBuffers[f], image);

		renderPass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), { 0,0 }, { size.width, size.height }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });


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
		

		
		if (!Command::SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(1) }, commandBuffer[f].getFence())) {
			continue;
		}

		

		Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                              // uint32_t               ImageIndex
		};
		if (!Presentation::PresentImage(queue->getHandle(), { commandBuffer[f].getSemaphore(1) }, { present_info })) {
			continue;
		}

	}
	d->end();
}

