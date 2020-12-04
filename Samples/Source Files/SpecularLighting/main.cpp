#include "Framework/Framework.h"
#include "AllHeaders.h"
#include "Common.h"
#include "VulkanDestroyer.h"
#include "Geometry/meshLoader.h"
#include "Geometry/meshExporter.h"
#include "Geometry/computationalMesh.h"

using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;
using namespace LavaCake::Core;

int main() {
	int nbFrames = 3;
	ErrorCheck::PrintError(true);
	Window w("LavaCake : Specular Lighting", 512, 512);
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
		commandBuffer[i].addSemaphore();
	}

	//vertex buffer
	//knot mesh
	auto knot = Load3DModelFromObjFile("Data/Models/mech.obj", true, true);
	Mesh_t* knot_mesh = new IndexedMesh<TRIANGLE>(knot.first.first, knot.first.second,knot.second);

	PolygonalMesh p((TriangleIndexedMesh*)knot_mesh);

	auto curvature = p.GaussianCurvature(0.08);

	float min = 1.0;
	float max = 0.0f;
	for (int i = 0; i < curvature.size(); i++) {
		curvature[i] = abs(curvature[i]);
		if (curvature[i] > max) {
			max = curvature[i];
		}

		if (curvature[i] < min) {
			min = curvature[i];
		}
	}
	Mesh_t* out = new IndexedMesh<TRIANGLE>(PC3);

	for (int i = 0; i < p.vertices.size(); i++) {
		float c = (curvature[i] - min) / (max - min);
		out->appendVertex({ 
			p.vertices[i]->p[0],
			p.vertices[i]->p[1],
			p.vertices[i]->p[2] ,
			c,
			c,
			c
			});

	}
	for (int i = 0; i < p.faces.size(); i++) {
		out->appendIndex(p.faces[i]->vertices[0]->indice);
		out->appendIndex(p.faces[i]->vertices[1]->indice);
		out->appendIndex(p.faces[i]->vertices[2]->indice);
	}

	exportToPly(out, "test.ply");

	VertexBuffer* v = new VertexBuffer({ knot_mesh });
	v->allocate(queue, commandBuffer[0]);

	//uniform buffer
	UniformBuffer* b = new UniformBuffer();
	mat4 proj = Helpers::PreparePerspectiveProjectionMatrix(static_cast<float>(size.width) / static_cast<float>(size.height),
		50.0f, 0.5f, 10.0f);
	mat4 modelView = Helpers::PrepareTranslationMatrix(0.0f, 0.0f, -4.0f);
	b->addVariable("modelView", &modelView);
	b->addVariable("projection", &proj);
	b->end();


	//PushConstant
	PushConstant* constant = new PushConstant();
	vec4f LigthPos = vec4f({ 0.f,4.f,0.7f,0.0 });
	constant->addVariable("LigthPos", &LigthPos);

	// Render pass
	RenderPass pass = RenderPass();
	GraphicPipeline* pipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));

	VertexShaderModule* vertex = new VertexShaderModule("Data/Shaders/SpecularLighting/shader.vert.spv");
	pipeline->setVextexShader(vertex);

	FragmentShaderModule* frag = new FragmentShaderModule("Data/Shaders/SpecularLighting/shader.frag.spv");
	pipeline->setFragmentModule(frag);
	pipeline->addPushContant(constant, VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->setVertices(v);
	pipeline->addUniformBuffer(b, VK_SHADER_STAGE_VERTEX_BIT, 0);

	pass.addSubPass({ pipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);
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

			modelView = modelView * Helpers::PrepareTranslationMatrix(0.0f, 0.0f,  - 4.0f);

			modelView = modelView * Helpers::PrepareRotationMatrix(-float(polars[0]), vec3f({ 0 , 1, 0 }));
			modelView = modelView * Helpers::PrepareRotationMatrix(float(polars[1]), vec3f({ 1 , 0, 0 }));
			//std::cout << w.m_mouse.position[0] << std::endl;
			b->setVariable("modelView", &modelView);
			lastMousePos = new vec2d({ mouse->position[0], mouse->position[1] });
		}
		else {
			lastMousePos = nullptr;
		}



		VkDevice logical = d->getLogicalDevice();
		VkQueue& present_queue = d->getPresentQueue()->getHandle();
		VkSwapchainKHR& swapchain = s->getHandle();
		

		SwapChainImage& image = s->AcquireImage();

		std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),																	// VkSemaphore            Semaphore
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
		pass.draw(commandBuffer[f].getHandle(), frameBuffers[f]->getHandle(), vec2u({ 0,0 }), vec2u({ size.width, size.height }), {{ 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 }});



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
