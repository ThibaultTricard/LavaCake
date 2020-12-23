#include "Framework/Framework.h"
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32 true
#include "glfw3.h"
#include "glfw3native.h"

using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;
using namespace LavaCake::Core;

int main() {

	Window w("LavaCake HelloWorld", 512, 512);

	
	Device* d = Device::getDevice();
	d->initDevices(0, 1, w.m_windowParams);
	SwapChain* s = SwapChain::getSwapChain();
	s->init();
	VkExtent2D size = s->size();
	Queue* queue = d->getGraphicQueue(0);
	Queue* presentQueue = d->getPresentQueue();
	CommandBuffer  commandBuffer;
	commandBuffer.addSemaphore();


	//We define the stride format we need for the mesh here 
	//each vertex is a 3D position followed by a RGB color
	vertexFormat format = vertexFormat({ POS3,COL3 });

	//we create a indexed triangle mesh with the desired format
	Mesh_t* triangle = new IndexedMesh<TRIANGLE>(format);

	//adding 3 vertices
	triangle->appendVertex({ -0.75f, 0.75f, 0.0f, 1.0f , 0.0f , 0.0f });
	triangle->appendVertex({ 0.75f,	0.75f , 0.0f, 0.0f , 1.0f	, 0.0f });
	triangle->appendVertex({ 0.0f , -0.75f, 0.0f, 0.0f , 0.0f	, 1.0f });


	// we link the 3 vertices to define a triangle
	triangle->appendIndex(0);
	triangle->appendIndex(1);
	triangle->appendIndex(2);


	//creating an allocating a vertex buffer
	VertexBuffer* triangle_vertex_buffer = new VertexBuffer({ triangle });
	triangle_vertex_buffer->allocate(queue, commandBuffer);

	
	
	VertexShaderModule* vertexShader = new VertexShaderModule("Data/Shaders/helloworld/shader.vert.spv");
	FragmentShaderModule* fragmentShader = new FragmentShaderModule("Data/Shaders/helloworld/shader.frag.spv");

	GraphicPipeline* pipeline = new GraphicPipeline(vec3f({ 0,0,0 }), vec3f({ float(size.width),float(size.height),1.0f }), vec2f({ 0,0 }), vec2f({ float(size.width),float(size.height) }));
	pipeline->setVextexShader(vertexShader);
	pipeline->setFragmentModule(fragmentShader);
	pipeline->setVertices(triangle_vertex_buffer);


	RenderPass* pass = new RenderPass();
	pass->addSubPass({ pipeline }, RenderPassFlag::SHOW_ON_SCREEN | RenderPassFlag::USE_COLOR | RenderPassFlag::USE_DEPTH | RenderPassFlag::OP_STORE_COLOR);
	pass->addDependencies(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass->addDependencies(0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
	pass->compile();

	FrameBuffer* frameBuffers = new FrameBuffer(s->size().width, s->size().height);
	pass->prepareOutputFrameBuffer(*frameBuffers);
	

	while (w.running()) {
		w.updateInput();
	
		VkDevice logical = d->getLogicalDevice();
		VkSwapchainKHR& swapchain = s->getHandle();
		SwapChainImage& image = s->AcquireImage();

		std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		commandBuffer.wait(2000000000);
		commandBuffer.resetFence();
		commandBuffer.beginRecord();

		
		

		pass->setSwapChainImage(*frameBuffers, image);

		pass->draw(commandBuffer.getHandle(), frameBuffers->getHandle(), vec2u({ 0,0 }), vec2u({ size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });

		commandBuffer.endRecord();

		
		if (!SubmitCommandBuffersToQueue(queue->getHandle(), wait_semaphore_infos, { commandBuffer.getHandle() }, { commandBuffer.getSemaphore(0) }, commandBuffer.getFence())) {
			continue;
		}


		PresentInfo present_info = {
			swapchain,                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                              // uint32_t               ImageIndex
		};

		if (!PresentImage(presentQueue->getHandle(), { commandBuffer.getSemaphore(0) }, { present_info })) {
			continue;
		}
	}
	d->end();
}