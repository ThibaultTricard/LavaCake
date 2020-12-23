#include "Framework/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"
#include "Geometry/meshLoader.h"

using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;
using namespace LavaCake::Core;

int main() {
	uint32_t nbFrames = 4;
	ErrorCheck::PrintError(true);
	Window w("LavaCake : Shadow", 1000, 800);
	Mouse* mouse = Mouse::getMouse();
	Device* d = Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	SwapChain* s = SwapChain::getSwapChain();
	s->init();
	VkDevice logical = d->getLogicalDevice();
	Queue* queue = d->getGraphicQueue(0);
	VkQueue& present_queue = d->getPresentQueue()->getHandle();

	VkExtent2D size = s->size();

	std::vector<CommandBuffer> commandBuffer = std::vector<CommandBuffer>(nbFrames);
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



	VertexBuffer* scene_vertex_buffer = new VertexBuffer({ plane_mesh, knot_mesh });
	scene_vertex_buffer->allocate(queue, commandBuffer[0]);

	VertexBuffer* plane_buffer = new VertexBuffer({ plane_mesh });
	plane_buffer->allocate(queue, commandBuffer[0]);


	//uniform buffer
	UniformBuffer* b = new UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = mat4({ 0.9981f, -0.0450f, 0.0412f, 0.0000f, 0.0000f, 0.6756f, 0.7373f, 0.0000f, -0.0610f, -0.7359f, 0.6743f, 0.0000f, -0.0000f, -0.0000f, -4.0000f, 1.0000f });
	mat4 lightView = mat4({ 1.0f,0.0f,0.0f,0.0f,0.0f,0.173648223f ,0.984807730f,0.0f,0.0f, -0.984807730f, 0.173648223f ,0.0f,0.0f,0.0f,-3.99999976f ,1.0f });
	b->addVariable("ligthView", &lightView);
	b->addVariable("modelView", &modelView);
	b->addVariable("projection", &proj);
	b->end();

	//PushConstant
	PushConstant* constant = new PushConstant();
	vec4f LigthPos = vec4f({ 0.f,4.f,0.7f,0.0 });
	constant->addVariable("LigthPos", &LigthPos);

	uint32_t shadowsize = 512;
	
	//frameBuffer
	FrameBuffer* shadow_map_buffer = new FrameBuffer(shadowsize, shadowsize);
	
	// Shadow pass
	RenderPass shadowMapPass = RenderPass();
	GraphicPipeline* shadowPipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(shadowsize),float(shadowsize),1.0f }), vec2f({ 0,0 }), vec2f({ float(shadowsize),float(shadowsize) }));

	VertexShaderModule* shadowVertex = new VertexShaderModule("Data/Shaders/Shadow/shadow.vert.spv");
	shadowPipeline->setVextexShader(shadowVertex);


	shadowPipeline->setVertices(scene_vertex_buffer);
	shadowPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);


	shadowMapPass.addSubPass({ shadowPipeline }, RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_DEPTH);
	shadowMapPass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	shadowMapPass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	shadowMapPass.compile();

	shadowMapPass.prepareOutputFrameBuffer(*shadow_map_buffer);
	//Render Pass
	RenderPass renderPass = RenderPass();
	GraphicPipeline* renderPipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	VertexShaderModule* renderVertex = new VertexShaderModule("Data/Shaders/Shadow/scene.vert.spv");
	renderPipeline->setVextexShader(renderVertex);

	FragmentShaderModule* renderFrag = new FragmentShaderModule("Data/Shaders/Shadow/scene.frag.spv");
	renderPipeline->setFragmentModule(renderFrag);

	
	renderPipeline->addPushContant(constant, VK_SHADER_STAGE_VERTEX_BIT);

	renderPipeline->setVertices(scene_vertex_buffer);
	renderPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);

	renderPipeline->addFrameBuffer(shadow_map_buffer, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	renderPass.addSubPass({ renderPipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);
	renderPass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.compile();


	std::vector<FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new FrameBuffer(s->size().width, s->size().height));
		renderPass.prepareOutputFrameBuffer(*frameBuffers[i]);
	}

	bool updateUniformBuffer = true;
	uint32_t f = 0;

	vec2d* lastMousePos = nullptr;

	vec2d polars = vec2d({ 0.0,0.0 });
	while (w.running()) {

		
		
		VkSwapchainKHR& swapchain = s->getHandle();
		VkExtent2D size = s->size();
		SwapChainImage& image = s->AcquireImage();

		std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
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

			modelView = modelView * Helpers::PrepareRotationMatrix(-float(polars[0]), vec3f({ 0 , 1, 0 }));
			modelView = modelView * Helpers::PrepareRotationMatrix(float(polars[1]), vec3f({ 1 , 0, 0 }));
			//std::cout << w.m_mouse.position[0] << std::endl;
			b->setVariable("modelView", &modelView);
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

		shadowMapPass.draw(commandBuffer[f].getHandle(), shadow_map_buffer->getHandle(), vec2u({ 0,0 }), vec2u({ shadowsize, shadowsize }));


		if (d->getPresentQueue()->getIndex() != d->getGraphicQueue(0)->getIndex()) {
			ImageTransition image_transition_before_drawing = {
				image.getImage(),													// VkImage              Image
				VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        CurrentAccess
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // VkAccessFlags        NewAccess
				VK_IMAGE_LAYOUT_UNDEFINED,                // VkImageLayout        CurrentLayout
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout        NewLayout
				d->getPresentQueue()->getIndex(),         // uint32_t             CurrentQueueFamily
				d->getGraphicQueue(0)->getIndex(),        // uint32_t             NewQueueFamily
				VK_IMAGE_ASPECT_COLOR_BIT                 // VkImageAspectFlags   Aspect
			};
			SetImageMemoryBarrier(commandBuffer[f].getHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
		}


		

	
		renderPass.setSwapChainImage(*frameBuffers[f], image);

		renderPass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), vec2u({ 0,0 }), vec2u({ size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });


		if (d->getPresentQueue()->getIndex() != d->getGraphicQueue(0)->getIndex()) {
			ImageTransition image_transition_before_drawing = {
				image.getImage(),														// VkImage              Image
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,				// VkAccessFlags        CurrentAccess
				VK_ACCESS_MEMORY_READ_BIT,									// VkAccessFlags        NewAccess
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,						// VkImageLayout        CurrentLayout
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,						// VkImageLayout        NewLayout
				d->getGraphicQueue(0)->getIndex(),          // uint32_t             CurrentQueueFamily
				d->getPresentQueue()->getIndex(),						// uint32_t             NewQueueFamily
				VK_IMAGE_ASPECT_COLOR_BIT										// VkImageAspectFlags   Aspect
			};
			SetImageMemoryBarrier(commandBuffer[f].getHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
		}

		commandBuffer[f].endRecord();
		

		
		if (!SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(1) }, commandBuffer[f].getFence())) {
			continue;
		}

		

		PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                              // uint32_t               ImageIndex
		};
		if (!PresentImage(queue->getHandle(), { commandBuffer[f].getSemaphore(1) }, { present_info })) {
			continue;
		}

	}
	d->end();
}

