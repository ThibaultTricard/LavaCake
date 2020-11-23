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
	int nbFrames = 3;
	ErrorCheck::PrintError(true);
	Window w("LavaCake : Post Process", 1000, 800);
	Mouse* mouse = Mouse::getMouse();
	Device* d = Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	SwapChain* s = SwapChain::getSwapChain();
	s->init();
	VkExtent2D size = s->size();
	Queue* queue = d->getGraphicQueue(0);
	VkQueue& present_queue = d->getPresentQueue()->getHandle();
	std::vector<CommandBuffer> commandBuffer = std::vector<CommandBuffer>(nbFrames);
	for (int i = 0; i < nbFrames; i++) {
		commandBuffer[i].addSemaphore();
	}

	std::pair<std::vector<float>, vertexFormat > sphere = Load3DModelFromObjFile("Data/Models/sphere.obj", true, false, false, true);
	Geometry::Mesh_t* sphere_mesh = new Geometry::Mesh<Geometry::TRIANGLE>(sphere.first, sphere.second);

	

	VertexBuffer* sphere_vertex_buffer = new VertexBuffer({ sphere_mesh });
	sphere_vertex_buffer->allocate(queue, commandBuffer[0]);

	//Skybox Data

	std::pair<std::vector<float>, vertexFormat > sky = Load3DModelFromObjFile("Data/Models/cube.obj", true, false, false, true);
	Geometry::Mesh_t* sky_mesh = new Geometry::Mesh<Geometry::TRIANGLE>(sky.first, sky.second);
	


	VertexBuffer* sky_vertex_buffer = new VertexBuffer({ sky_mesh });
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

	VertexBuffer* quad_vertex_buffer = new VertexBuffer({ quad });
	quad_vertex_buffer->allocate(queue, commandBuffer[0]);
	

	//Uniform Buffer
	UniformBuffer* uniforms = new UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	uniforms->addVariable("modelView", &modelView);
	uniforms->addVariable("projection", &proj);
	uniforms->end();

	//SkyBox texture
	CubeMap* skyCubeMap = new CubeMap("Data/Textures/Skansen/", 4);
	skyCubeMap->allocate(queue, commandBuffer[0]);



	//Time PushConstant
	PushConstant* timeConstant = new PushConstant();
	float time = 0.0;
	timeConstant->addVariable("Time", new v<float, 1>({ time }));


	//camera PushConstant
	PushConstant* cameraConstant = new PushConstant();
	vec4f camera = vec4f({ 0.f,0.f,4.f,1.0 });
	cameraConstant->addVariable("camera", &camera);

	//Color Attachment
	Attachment* colorAttachemnt = new Attachment(size.width, size.height, s->imageFormat(), attachmentType::COLOR_ATTACHMENT);
	colorAttachemnt->allocate();

	//Render Pass
	RenderPass renderPass = RenderPass( );

	
	GraphicPipeline* sphereRenderPipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	VertexShaderModule* sphereVertex = new VertexShaderModule("Data/Shaders/PostProcess/model.vert.spv");
	sphereRenderPipeline->setVextexShader(sphereVertex);

	FragmentShaderModule* sphereFrag = new FragmentShaderModule("Data/Shaders/PostProcess/model.frag.spv");
	sphereRenderPipeline->setFragmentModule(sphereFrag);

	sphereRenderPipeline->setVertices(sphere_vertex_buffer);
	sphereRenderPipeline->addUniformBuffer(uniforms, VK_SHADER_STAGE_VERTEX_BIT, 0);
	sphereRenderPipeline->addTextureBuffer(skyCubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	sphereRenderPipeline->addPushContant(cameraConstant, VK_SHADER_STAGE_FRAGMENT_BIT);


	GraphicPipeline* skyRenderPipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));

	VertexShaderModule* skyVertex = new VertexShaderModule("Data/Shaders/PostProcess/skybox.vert.spv");
	skyRenderPipeline->setVextexShader(skyVertex);

	FragmentShaderModule* skyFrag = new FragmentShaderModule("Data/Shaders/PostProcess/skybox.frag.spv");
	skyRenderPipeline->setFragmentModule(skyFrag);


	skyRenderPipeline->setVertices(sky_vertex_buffer);
	skyRenderPipeline->addUniformBuffer(uniforms, VK_SHADER_STAGE_VERTEX_BIT, 0);
	skyRenderPipeline->addTextureBuffer(skyCubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	skyRenderPipeline->SetCullMode(VK_CULL_MODE_FRONT_BIT);

	renderPass.addSubPass({ sphereRenderPipeline,skyRenderPipeline }, RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);

	GraphicPipeline* postProcessPipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	VertexShaderModule* postProcessVertex = new VertexShaderModule("Data/Shaders/PostProcess/postprocess.vert.spv");
	postProcessPipeline->setVextexShader(postProcessVertex);

	FragmentShaderModule* postProcessFrag = new FragmentShaderModule("Data/Shaders/PostProcess/postprocess.frag.spv");
	postProcessPipeline->setFragmentModule(postProcessFrag);

	postProcessPipeline->addPushContant(timeConstant, VK_SHADER_STAGE_FRAGMENT_BIT);
	postProcessPipeline->setVertices(quad_vertex_buffer);
	

	
	postProcessPipeline->addAttachment(colorAttachemnt, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	renderPass.addSubPass({ postProcessPipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::OP_STORE_COLOR | RenderPassFlag::ADD_INPUT, { 0 });

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

	std::vector<FrameBuffer*> frameBuffers;
	for (int i = 0; i < nbFrames; i++) {
		frameBuffers.push_back(new FrameBuffer(s->size().width, s->size().height));
		renderPass.prepareOutputFrameBuffer(*frameBuffers[i]);
	}

	bool updateUniformBuffer = true;
	int f = 0;
	vec2d* lastMousePos = nullptr;

	vec2d polars = vec2d({ 0.0,0.0 });
	while (w.running()) {
		w.updateInput();
		f++;
		f = f % nbFrames;

		VkDevice logical = d->getLogicalDevice();
		VkSwapchainKHR& swapchain = s->getHandle();
		VkExtent2D size = s->size();

		SwapChainImage& image = s->AcquireImage();
		std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),																	// VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		time +=0.001f;
		timeConstant->setVariable("Time", new v<float, 1>({ time }));

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
			uniforms->setVariable("modelView", &modelView);
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
		renderPass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), vec2u({ 0,0 }), vec2u({ size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } , { 0.1f, 0.2f, 0.3f, 1.0f } });



		commandBuffer[f].endRecord();



		if (!SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { commandBuffer[f].getHandle() }, { commandBuffer[f].getSemaphore(0) }, commandBuffer[f].getFence())) {
			continue;
		}

		PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                              // uint32_t               ImageIndex
		};
		if (!PresentImage(present_queue, { commandBuffer[f].getSemaphore(0) }, { present_info })) {
			continue;
		}
	}

	d->end();
}

