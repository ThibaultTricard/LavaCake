#include "Framework/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"
#include "Geometry/meshLoader.h"

using namespace LavaCake;
using namespace LavaCake::Geometry;

int main() {
	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Post Process", 1000, 800);
	Framework::Mouse* mouse = Framework::Mouse::getMouse();
	Framework::Device* d = Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	LavaCake::Framework::SwapChain* s = LavaCake::Framework::SwapChain::getSwapChain();
	s->init();
	VkExtent2D size = s->size();
	Framework::Queue* queue = d->getGraphicQueue(0);
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	std::vector<Framework::CommandBuffer> commandBuffer = std::vector<Framework::CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
	}

	std::pair<std::vector<float>, vertexFormat > sphere = Load3DModelFromObjFile("Data/Models/sphere.obj", true, false, false, true);
	Geometry::Mesh_t* sphere_mesh = new Geometry::Mesh<Geometry::TRIANGLE>(sphere.first, sphere.second);

	

	Framework::VertexBuffer* sphere_vertex_buffer = new Framework::VertexBuffer({ sphere_mesh });
	sphere_vertex_buffer->allocate(queue, commandBuffer[0]);

	//Skybox Data

	std::pair<std::vector<float>, vertexFormat > sky = Load3DModelFromObjFile("Data/Models/cube.obj", true, false, false, true);
	Geometry::Mesh_t* sky_mesh = new Geometry::Mesh<Geometry::TRIANGLE>(sky.first, sky.second);
	


	Framework::VertexBuffer* sky_vertex_buffer = new Framework::VertexBuffer({ sky_mesh });
	sky_vertex_buffer->allocate(queue, commandBuffer[0]);


	//PostProcessQuad
	Geometry::Mesh_t* quad = new Geometry::IndexedMesh<Geometry::TRIANGLE>(Geometry::P3UV);

	quad->appendVertex({ -1.0,-1.0,0.0,0.0,0.0 });
	quad->appendVertex({ -1.0, 1.0,0.0,0.0,1.0 });
	quad->appendVertex({ 1.0, 1.0,0.0,1.0,1.0 });
	quad->appendVertex({ 1.0,-1.0,0.0,1.0,0.0 });

	quad->appendIndex(0);
	quad->appendIndex(1);
	quad->appendIndex(2);

	quad->appendIndex(2);
	quad->appendIndex(3);
	quad->appendIndex(0);

	Framework::VertexBuffer* quad_vertex_buffer = new Framework::VertexBuffer({ quad });
	quad_vertex_buffer->allocate(queue, commandBuffer[0]);
	

	//Uniform Buffer
	Framework::UniformBuffer* uniforms = new Framework::UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	uniforms->addVariable("modelView", modelView);
	uniforms->addVariable("projection", proj);
	uniforms->end();

	//SkyBox texture
	Framework::CubeMap* skyCubeMap = new Framework::CubeMap("Data/Textures/Skansen/", 4);
	skyCubeMap->allocate(queue->getHandle(), commandBuffer[0].getHandle());



	//Time PushConstant
	Framework::PushConstant* timeConstant = new Framework::PushConstant();
	float time = 0.0;
	timeConstant->addVariable("Time", time);


	//camera PushConstant
	Framework::PushConstant* cameraConstant = new Framework::PushConstant();
	vec4f camera = { 0.f,0.f,4.f,1.0 };
	cameraConstant->addVariable("camera", camera);

	//Color Attachment
	Framework::Attachment* colorAttachemnt = new Framework::Attachment(size.width, size.height, s->imageFormat(), Framework::attachmentType::COLOR_ATTACHMENT);
	colorAttachemnt->allocate();

	//Render Pass
	Framework::RenderPass renderPass = Framework::RenderPass( );

	
	Framework::GraphicPipeline* sphereRenderPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(size.width),float(size.height),1.0f }, { 0,0 }, { float(size.width),float(size.height) });
	Framework::VertexShaderModule* sphereVertex = new Framework::VertexShaderModule("Data/Shaders/PostProcess/model.vert.spv");
	sphereRenderPipeline->setVextexShader(sphereVertex);

	Framework::FragmentShaderModule* sphereFrag = new Framework::FragmentShaderModule("Data/Shaders/PostProcess/model.frag.spv");
	sphereRenderPipeline->setFragmentModule(sphereFrag);

	sphereRenderPipeline->setVertices(sphere_vertex_buffer);
	sphereRenderPipeline->addUniformBuffer(uniforms, VK_SHADER_STAGE_VERTEX_BIT, 0);
	sphereRenderPipeline->addTextureBuffer(skyCubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	sphereRenderPipeline->addPushContant(cameraConstant, VK_SHADER_STAGE_FRAGMENT_BIT);


	Framework::GraphicPipeline* skyRenderPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(size.width),float(size.height),1.0f }, { 0,0 }, { float(size.width),float(size.height) });

	Framework::VertexShaderModule* skyVertex = new Framework::VertexShaderModule("Data/Shaders/PostProcess/skybox.vert.spv");
	skyRenderPipeline->setVextexShader(skyVertex);

	Framework::FragmentShaderModule* skyFrag = new Framework::FragmentShaderModule("Data/Shaders/PostProcess/skybox.frag.spv");
	skyRenderPipeline->setFragmentModule(skyFrag);


	skyRenderPipeline->setVertices(sky_vertex_buffer);
	skyRenderPipeline->addUniformBuffer(uniforms, VK_SHADER_STAGE_VERTEX_BIT, 0);
	skyRenderPipeline->addTextureBuffer(skyCubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	skyRenderPipeline->SetCullMode(VK_CULL_MODE_FRONT_BIT);

	renderPass.addSubPass({ sphereRenderPipeline,skyRenderPipeline }, Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);

	Framework::GraphicPipeline* postProcessPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(size.width),float(size.height),1.0f }, { 0,0 }, { float(size.width),float(size.height) });
	Framework::VertexShaderModule* postProcessVertex = new Framework::VertexShaderModule("Data/Shaders/PostProcess/postprocess.vert.spv");
	postProcessPipeline->setVextexShader(postProcessVertex);

	Framework::FragmentShaderModule* postProcessFrag = new Framework::FragmentShaderModule("Data/Shaders/PostProcess/postprocess.frag.spv");
	postProcessPipeline->setFragmentModule(postProcessFrag);

	postProcessPipeline->addPushContant(timeConstant, VK_SHADER_STAGE_FRAGMENT_BIT);
	postProcessPipeline->setVertices(quad_vertex_buffer);
	

	
	postProcessPipeline->addAttachment(colorAttachemnt, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	renderPass.addSubPass({ postProcessPipeline }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::OP_STORE_COLOR | Framework::RenderPassFlag::ADD_INPUT, { 0 });

	renderPass.addDependencies(
		0,
		1,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT 
	);
	renderPass.addDependencies(
		VK_SUBPASS_EXTERNAL,  
		1, 
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
		VK_ACCESS_MEMORY_READ_BIT, 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
		VK_DEPENDENCY_BY_REGION_BIT 
	);
	renderPass.addDependencies(
		1,
		VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
		VK_ACCESS_MEMORY_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT 
	);
	renderPass.compile();

	std::vector<Framework::FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new Framework::FrameBuffer(s->size().width, s->size().height));
		renderPass.prepareOutputFrameBuffer(*frameBuffers[i]);
	}

	bool updateUniformBuffer = true;
	int f = 0;
	vec2d* lastMousePos = nullptr;

	vec2d polars = { 0.0,0.0 };
	while (w.running()) {
		w.updateInput();
		f++;
		f = f % nbFrames;

		VkDevice logical = d->getLogicalDevice();
		VkSwapchainKHR& swapchain = s->getHandle();
		VkExtent2D size = s->size();

		Framework::SwapChainImage& image = s->AcquireImage();
		std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),																	// VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		time +=0.001f;
		timeConstant->setVariable("Time", time);

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
			uniforms->setVariable("modelView", modelView);
			lastMousePos = new vec2d({ mouse->position[0], mouse->position[1] });
		}
		else {
			lastMousePos = nullptr;
		}



		commandBuffer[f].wait(2000000000);
		commandBuffer[f].resetFence();
		commandBuffer[f].beginRecord();


		


		if (updateUniformBuffer) {
			uniforms->update(commandBuffer[f].getHandle());
			updateUniformBuffer = false;
		}



		


		
		renderPass.setSwapChainImage(*frameBuffers[f], image);
		renderPass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), { 0,0 }, { size.width, size.height }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } , { 0.1f, 0.2f, 0.3f, 1.0f } });



		commandBuffer[f].endRecord();



		if (!Command::SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(0) }, commandBuffer[f].getFence())) {
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

