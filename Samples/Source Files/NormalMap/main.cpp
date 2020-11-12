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
	Framework::Window w("LavaCake : Bump Mapping", 500, 500);
	Framework::Mouse* mouse = Framework::Mouse::getMouse();

	LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
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

	//Normal map
	Framework::TextureBuffer* normalMap = new Framework::TextureBuffer("Data/Textures/normal_map.png", 4);
	normalMap->allocate(queue->getHandle(), commandBuffer[0].getHandle());

	//vertex buffer
	//knot mesh
	std::pair<std::vector<float>, vertexFormat > ice = Load3DModelFromObjFile("Data/Models/ice.obj", true,true, true, true);
	Geometry::Mesh_t* ice_mesh = new Geometry::Mesh<Geometry::TRIANGLE>(ice.first, ice.second);



	Framework::VertexBuffer* v = new Framework::VertexBuffer({ ice_mesh });
	v->allocate(queue, commandBuffer[0]);

	//uniform buffer
	Framework::UniformBuffer* b = new Framework::UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	b->addVariable("modelView", modelView);
	b->addVariable("projection", proj);
	b->end();


	//PushConstant
	Framework::PushConstant* constant = new Framework::PushConstant();
	vec4f LigthPos = vec4f({ 0.f,4.f,0.7f,0.0 });
	constant->addVariable("LigthPos", LigthPos);

	// Render pass
	Framework::RenderPass pass = Framework::RenderPass();
	Framework::GraphicPipeline* pipeline = new Framework::GraphicPipeline({ 0,0,0 }, { float(size.width),float(size.height),1.0f }, { 0,0 }, { float(size.width),float(size.height) });

	Framework::VertexShaderModule* vertex = new Framework::VertexShaderModule("Data/Shaders/NormalMap/shader.vert.spv");
	pipeline->setVextexShader(vertex);


	Framework::FragmentShaderModule* frag = new Framework::FragmentShaderModule("Data/Shaders/NormalMap/shader.frag.spv");
	pipeline->setFragmentModule(frag);

	pipeline->setVertices(v);
	pipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);
	pipeline->addTextureBuffer(normalMap, VK_SHADER_STAGE_FRAGMENT_BIT,1);
	pipeline->addPushContant(constant, VK_SHADER_STAGE_FRAGMENT_BIT);

	pass.addSubPass({ pipeline }, Framework::RenderPassFlag::SHOW_ON_SCREEN | Framework::RenderPassFlag::USE_COLOR | Framework::RenderPassFlag::USE_DEPTH | Framework::RenderPassFlag::OP_STORE_COLOR);
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

	vec2d polars = { 0.0,0.0 };

	while (w.running()) {
		w.updateInput();
		f++;
		f = f % nbFrames;

		VkDevice logical = d->getLogicalDevice();
		VkQueue& present_queue = d->getPresentQueue()->getHandle();
		Framework::SwapChainImage& image = s->AcquireImage();

		std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),											        // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
		});

		VkSwapchainKHR& swapchain = s->getHandle();
		VkExtent2D size = s->size();

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


		pass.setSwapChainImage(*frameBuffers[f], image);


		pass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), { 0,0 }, { size.width, size.height }, { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });



		if (!LavaCake::Command::EndCommandBufferRecordingOperation(commandBuffer[f].getHandle())) {
			continue;
		}


		
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

