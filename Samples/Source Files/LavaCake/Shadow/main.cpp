#include "LavaCake/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"

using namespace LavaCake;

int main() {
	uint32_t nbFrames = 4;
	Framework::ErrorCheck::PrintError(true);
	Framework::Window w("LavaCake : Shadow", 0, 0, 512, 512);

	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	d->prepareFrames(nbFrames);

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
	scene_vertex_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().CommandBuffer);

	Framework::VertexBuffer* plane_buffer = new Framework::VertexBuffer({ plane_mesh }, { 3,3 });
	plane_buffer->allocate(*d->getPresentQueue(), d->getFrameRessources()->front().CommandBuffer);


	//uniform buffer
	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::Transformation::PreparePerspectiveProjectionMatrix(static_cast<float>(w.m_windowSize[0]) / static_cast<float>(w.m_windowSize[1]),
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

	int shadowsize = 512;
	
	//frameBuffer
	Framework::FrameBuffer* shadow_map_buffer = new Framework::FrameBuffer(shadowsize, shadowsize, d->getSwapChain().depthFormat());
	shadow_map_buffer->allocate();
	
	// Shadow pass
	Framework::RenderPass shadowMapPass = Framework::RenderPass(Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_DEPTH);
	Framework::GraphicPipeline* shadowPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(shadowsize),float(shadowsize),1.0f }, { 0,0 }, { float(shadowsize),float(shadowsize) });

	Framework::VertexShaderModule* shadowVertex = new Framework::VertexShaderModule("Data/Shaders/11 Lighting/05 Adding shadows to the scene/shadow.vert.spv");
	shadowPipeline->setVextexShader(shadowVertex);


	shadowPipeline->setVeritices(scene_vertex_buffer);
	shadowPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);


	shadowMapPass.addSubPass(shadowPipeline);
	shadowMapPass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	shadowMapPass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	shadowMapPass.compile();

	shadow_map_buffer->setInputRenderPass(shadowMapPass.getHandle());

	//Render Pass
	Framework::RenderPass renderPass = Framework::RenderPass(Framework::RenderPassFlag::SHOW_ON_SCREEN| Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR );
	Framework::GraphicPipeline* renderPipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]),1.0f }, { 0,0 }, { float(w.m_windowSize[0]),float(w.m_windowSize[1]) });
	Framework::VertexShaderModule* renderVertex = new Framework::VertexShaderModule("Data/Shaders/11 Lighting/05 Adding shadows to the scene/scene.vert.spv");
	renderPipeline->setVextexShader(renderVertex);

	Framework::FragmentShaderModule* renderFrag = new Framework::FragmentShaderModule("Data/Shaders/11 Lighting/05 Adding shadows to the scene/scene.frag.spv");
	renderPipeline->setFragmentModule(renderFrag);

	
	renderPipeline->addPushContant(constant, VK_SHADER_STAGE_VERTEX_BIT);

	renderPipeline->setVeritices(scene_vertex_buffer);
	renderPipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);

	renderPipeline->addTextureBuffer(shadow_map_buffer, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	renderPass.addSubPass(renderPipeline);
	renderPass.addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	renderPass.compile();


	w.Show();
	bool updateUniformBuffer = true;
	uint32_t f = 0;
	while (w.m_loop) {

		Buffer::FrameResources& frame = d->getFrameRessources()->at(f);
		VkCommandBuffer commandbuffer = frame.CommandBuffer;
		VkDevice logical = d->getLogicalDevice();
		VkQueue& graphics_queue = d->getGraphicQueue(0)->getHandle();
		VkQueue& present_queue = d->getPresentQueue()->getHandle();


		w.UpdateInput();
		f++;
		f = f % nbFrames;

		if (w.m_mouse.m_actionPerformed) {
			updateUniformBuffer = true;
			modelView = Helpers::Transformation::Identity();

			modelView = modelView * Helpers::Transformation::PrepareTranslationMatrix(0.0f, 0.0f, w.m_mouse.m_wheel.distance - 4.0f);

			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(-float(w.m_mouse.m_position.x) / float(w.m_windowSize[0]) * 360, { 0 , 1, 0 });
			modelView = modelView * Helpers::Transformation::PrepareRotationMatrix(float(w.m_mouse.m_position.y) / float(w.m_windowSize[1]) * 360, { 1 , 0, 0 });

			b->setVariable("modelView", modelView);
		}


		if (!Fence::WaitForFences(logical, { *frame.DrawingFinishedFence }, false, 2000000000)) {
			continue;
		}
		if (!Fence::ResetFences(logical, { *frame.DrawingFinishedFence })) {
			continue;
		}

		
		if (!Command::BeginCommandBufferRecordingOperation(commandbuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
			continue;
		}


		if (updateUniformBuffer) {
			b->update(commandbuffer);
			updateUniformBuffer = false;
		}



		VkSwapchainKHR& swapchain = d->getSwapChain().getHandle();
		std::vector<VkImageView>& swapchain_image_views = d->getSwapChain().getImageView();
		VkSemaphore  image_acquired_semaphore = *frame.ImageAcquiredSemaphore;
		VkImageView depth_attachment = *frame.DepthAttachment;
		uint32_t image_index;
		VkExtent2D size = d->getSwapChain().size();


		shadowMapPass.draw(commandbuffer, shadow_map_buffer->getFrameBuffer(), { 0,0 }, { int(shadowsize), int(shadowsize) });


		if (d->getPresentQueue()->getIndex() != d->getGraphicQueue(0)->getIndex()) {
			Image::ImageTransition image_transition_before_drawing = {
				d->getSwapChain().getImages()[image_index],						// VkImage              Image
				VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        CurrentAccess
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // VkAccessFlags        NewAccess
				VK_IMAGE_LAYOUT_UNDEFINED,                // VkImageLayout        CurrentLayout
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout        NewLayout
				d->getPresentQueue()->getIndex(),         // uint32_t             CurrentQueueFamily
				d->getGraphicQueue(0)->getIndex(),        // uint32_t             NewQueueFamily
				VK_IMAGE_ASPECT_COLOR_BIT                 // VkImageAspectFlags   Aspect
			};
			Image::SetImageMemoryBarrier(frame.CommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
		}


		if (!Swapchain::AcquireSwapchainImage(logical, swapchain, image_acquired_semaphore, VK_NULL_HANDLE, image_index)) {
			continue;
		}

		std::vector<VkImageView> attachments = { swapchain_image_views[image_index] };
		if (VK_NULL_HANDLE != depth_attachment) {
			attachments.push_back(depth_attachment);
		}
		InitVkDestroyer(logical, frame.Framebuffer);
		if (!Buffer::CreateFramebuffer(logical, renderPass.getHandle(), attachments, size.width, size.height, 1, *frame.Framebuffer)) {
			continue;
		}


		renderPass.draw(commandbuffer, *frame.Framebuffer, { 0,0 }, { int(size.width), int(size.height) }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });


		if (d->getPresentQueue()->getIndex() != d->getGraphicQueue(0)->getIndex()) {
			Image::ImageTransition image_transition_before_drawing = {
				d->getSwapChain().getImages()[image_index],	// VkImage              Image
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,				// VkAccessFlags        CurrentAccess
				VK_ACCESS_MEMORY_READ_BIT,									// VkAccessFlags        NewAccess
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,						// VkImageLayout        CurrentLayout
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,						// VkImageLayout        NewLayout
				d->getGraphicQueue(0)->getIndex(),          // uint32_t             CurrentQueueFamily
				d->getPresentQueue()->getIndex(),						// uint32_t             NewQueueFamily
				VK_IMAGE_ASPECT_COLOR_BIT										// VkImageAspectFlags   Aspect
			};
			Image::SetImageMemoryBarrier(frame.CommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
		}

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

