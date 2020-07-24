#include "LavaCake/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"

using namespace LavaCake;

int main() {
	int nbFrames = 3;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Post Process", 0, 0, 1000, 800);

	Framework::Device* d = Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	d->prepareFrames(nbFrames);


	// Sphere Data
	Helpers::Mesh::Mesh* sphere = new Helpers::Mesh::Mesh();
	if (!Load3DModelFromObjFile("Data/Models/sphere.obj", true, false, false, true, *sphere)) {
		return false;
	}

	Framework::VertexBuffer* sphere_vertex_buffer = new Framework::VertexBuffer({ sphere }, { 3,3 });
	sphere_vertex_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().CommandBuffer);

	//Skybox Data
	Helpers::Mesh::Mesh* sky = new Helpers::Mesh::Mesh();
	if (!Load3DModelFromObjFile("Data/Models/cube.obj", false, false, false, true, *sky)) {
		return false;
	}

	Framework::VertexBuffer* sky_vertex_buffer = new Framework::VertexBuffer({ sky }, { 3 });
	sky_vertex_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().CommandBuffer);


	//PostProcessQuad
	Helpers::Mesh::Mesh* quad = new Helpers::Mesh::Mesh();
	Helpers::Mesh::preparePostProcessQuad(*quad);
	Framework::VertexBuffer* quad_vertex_buffer = new Framework::VertexBuffer({ quad }, { 3 });
	quad_vertex_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().CommandBuffer);
	

	//Uniform Buffer
	Framework::UniformBuffer* uniforms = new Framework::UniformBuffer();
	mat4 proj = Helpers::Transformation::PreparePerspectiveProjectionMatrix(static_cast<float>(w.m_windowSize[0]) / static_cast<float>(w.m_windowSize[1]),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = Helpers::Transformation::PrepareTranslationMatrix(0.0f, 0.0f, 4.0f);
	uniforms->addVariable("modelView", modelView);
	uniforms->addVariable("projection", proj);
	uniforms->end();

	//SkyBox texture
	Framework::CubeMap* skyCubeMap = new Framework::CubeMap("Data/Textures/Skansen/", 4);
	skyCubeMap->allocate();



	//Time PushConstant
	Framework::PushConstant* timeConstant = new Framework::PushConstant();
	float time = 0.0;
	timeConstant->addVariable("Time", time);


	//camera PushConstant
	Framework::PushConstant* cameraConstant = new Framework::PushConstant();
	vec4f camera = { 0.f,0.f,4.f,1.0 };
	cameraConstant->addVariable("camera", camera);

	//Color Attachment
	Framework::Attachment* colorAttachemnt = new Framework::Attachment(w.m_windowSize[0], w.m_windowSize[1], d->getSwapChain().imageFormat());
	colorAttachemnt->allocate();

	//Render Pass
	Framework::RenderPass renderPass = Framework::RenderPass( );

	
	Framework::GraphicPipeline* sphereRenderPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });
	Framework::VertexShaderModule* sphereVertex = new Framework::VertexShaderModule("Data/Shaders/12 Advanced Rendering Techniques/06 Using input attachment for color correction postprocess effect/model.vert.spv");
	sphereRenderPipeline->setVextexShader(sphereVertex);

	Framework::FragmentShaderModule* sphereFrag = new Framework::FragmentShaderModule("Data/Shaders/12 Advanced Rendering Techniques/06 Using input attachment for color correction postprocess effect/model.frag.spv");
	sphereRenderPipeline->setFragmentModule(sphereFrag);

	sphereRenderPipeline->setVeritices(sphere_vertex_buffer);
	sphereRenderPipeline->addUniformBuffer(uniforms, VK_SHADER_STAGE_VERTEX_BIT, 0);
	sphereRenderPipeline->addTextureBuffer(skyCubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	sphereRenderPipeline->addPushContant(cameraConstant, VK_SHADER_STAGE_FRAGMENT_BIT);


	Framework::GraphicPipeline* skyRenderPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });

	Framework::VertexShaderModule* skyVertex = new Framework::VertexShaderModule("Data/Shaders/12 Advanced Rendering Techniques/06 Using input attachment for color correction postprocess effect/skybox.vert.spv");
	skyRenderPipeline->setVextexShader(skyVertex);

	Framework::FragmentShaderModule* skyFrag = new Framework::FragmentShaderModule("Data/Shaders/12 Advanced Rendering Techniques/06 Using input attachment for color correction postprocess effect/skybox.frag.spv");
	skyRenderPipeline->setFragmentModule(skyFrag);


	skyRenderPipeline->setVeritices(sky_vertex_buffer);
	skyRenderPipeline->addUniformBuffer(uniforms, VK_SHADER_STAGE_VERTEX_BIT, 0);
	skyRenderPipeline->addTextureBuffer(skyCubeMap, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	skyRenderPipeline->SetCullMode(VK_CULL_MODE_FRONT_BIT);

	renderPass.addSubPass({ sphereRenderPipeline,skyRenderPipeline }, Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);

	Framework::GraphicPipeline* postProcessPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });
	Framework::VertexShaderModule* postProcessVertex = new Framework::VertexShaderModule("Data/Shaders/12 Advanced Rendering Techniques/06 Using input attachment for color correction postprocess effect/postprocess.vert.spv");
	postProcessPipeline->setVextexShader(postProcessVertex);

	Framework::FragmentShaderModule* postProcessFrag = new Framework::FragmentShaderModule("Data/Shaders/12 Advanced Rendering Techniques/06 Using input attachment for color correction postprocess effect/postprocess.frag.spv");
	postProcessPipeline->setFragmentModule(postProcessFrag);

	postProcessPipeline->addPushContant(timeConstant, VK_SHADER_STAGE_FRAGMENT_BIT);
	postProcessPipeline->setVeritices(quad_vertex_buffer);
	

	
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



	w.Show();
	bool updateUniformBuffer = true;
	int f = 0;
	while (w.m_loop) {
		w.UpdateInput();
		f++;
		f = f % nbFrames;


		time +=0.001f;
		timeConstant->setVariable("Time", time);

		if (w.m_mouse.m_actionPerformed) {
			updateUniformBuffer = true;
			modelView = Helpers::Transformation::Identity();

			modelView = modelView * Helpers::Transformation::PrepareTranslationMatrix(0.0f, 0.0f, w.m_mouse.m_wheel.distance - 4.0f);

			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(-float(w.m_mouse.m_position.x) / float(w.m_windowSize[0]) * 360 - 180, { 0 , 1, 0 });
			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(float(w.m_mouse.m_position.y) / float(w.m_windowSize[1]) * 360 - 180, { 1 , 0, 0 });

			uniforms->setVariable("modelView", modelView);
		}



		Buffer::FrameResources& frame = d->getFrameRessources()->at(f);
		VkCommandBuffer commandbuffer = frame.CommandBuffer;
		VkDevice logical = d->getLogicalDevice();
		VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();
		VkQueue& present_queue = d->getPresentQueue()->getHandle();

		if (!Fence::WaitForFences(logical, { *frame.DrawingFinishedFence }, false, 2000000000)) {
			continue;
		}
		if (!Fence::ResetFences(logical, { *frame.DrawingFinishedFence })) {
			continue;
		}

		InitVkDestroyer(logical, frame.Framebuffer);
		if (!Command::BeginCommandBufferRecordingOperation(commandbuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
			continue;
		}


		if (updateUniformBuffer) {
			uniforms->update(commandbuffer);
			updateUniformBuffer = false;
		}



		VkSwapchainKHR& swapchain = d->getSwapChain().getHandle();
		std::vector<VkImageView>& swapchain_image_views = d->getSwapChain().getImageView();
		VkSemaphore  image_acquired_semaphore = *frame.ImageAcquiredSemaphore;
		VkImageView depth_attachment = *frame.DepthAttachment;
		uint32_t image_index;
		VkExtent2D size = d->getSwapChain().size();

		if (!Swapchain::AcquireSwapchainImage(logical, swapchain, image_acquired_semaphore, VK_NULL_HANDLE, image_index)) {
			continue;
		}

		std::vector<VkImageView> attachments = {colorAttachemnt->getImageView(),depth_attachment, swapchain_image_views[image_index]  };
		if (!Buffer::CreateFramebuffer(logical, renderPass.getHandle(), attachments, size.width, size.height, 1, *frame.Framebuffer)) {
			continue;
		}


		renderPass.draw(frame.CommandBuffer, *frame.Framebuffer, { 0,0 }, { int(size.width), int(size.height) }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } , { 0.1f, 0.2f, 0.3f, 1.0f } });



		if (!LavaCake::Command::EndCommandBufferRecordingOperation(commandbuffer)) {
			continue;
		}


		std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image_acquired_semaphore,                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT // VkPipelineStageFlags   WaitingStage
			});
		if (!Command::SubmitCommandBuffersToQueue(graphics_queue, wait_semaphore_infos, { commandbuffer }, { *frame.ReadyToPresentSemaphore }, *frame.DrawingFinishedFence)) {
			continue;
		}

		Presentation::PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image_index                                   // uint32_t               ImageIndex
		};
		if (!Presentation::PresentImage(present_queue, { *frame.ReadyToPresentSemaphore }, { present_info })) {
			continue;
		}
	}

	d->end();
}

