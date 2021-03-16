# Hello World

## Creating a Window

LavaCake use glfw3 as a window manager.
glfw is wrapped in the Window class : 

to create a window use the following command :

`LavaCake::Framework::Window window(title, width, height);`

* title : the name of your app.
* width : the width of your app.
* height : the height of your app.

The windows should open itself after that

To know if your window is still alive call the function window.running().
this function return true when the window is alive, and false when the windows must be killed.

To refresh the Window call window.updateInput()

A typical code to do a render using the LavaCake window manager should look like this : 


```
#include "LavaCake/Window.h"
using namespace LavaCake::Framework;


int main() {

	Window window("My window", 500, 500);
        //Vulkan Initialisation
        

        //Renderering setup


	while (window.running()) {
	window.updateInput();

	//render loop

	}
}
```
## Vulkan initialisation

To initialise vulkan in LavaCake you need to get a reference to the Device singleton provided by LavaCake and initialise it.\
This singleton holds references to Physical and Logical Device, Queues, Surface and Instances.


```
Device* d = Device::getDevice();
d->initDevices(0, 1, window.m_windowParams);
```


Then we need initialise the get a reference to the Swapchain singleton and initialise it.\
This singleton holds the rendering parameter such as the output size, the color and depth format etc...\
This singleton will also provide the swapchain images requiered to draw on the screen.

```
SwapChain* s = SwapChain::getSwapChain();
s->init();
```

## Renderering setup


## Final code

```
#include "Framework/Framework.h"


using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;
using namespace LavaCake::Core;


int main() {

	Window w("LavaCake HelloWorld", 512, 512);

	
	Device* d = Device::getDevice();
	d->initDevices(0, 1, window.m_windowParams);
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
	pipeline->setVextexModule(vertexShader);
	pipeline->setFragmentModule(fragmentShader);
	pipeline->setVertices(triangle_vertex_buffer);


	SubpassAttachment SA;
	SA.showOnScreen = true;
	SA.nbColor = 1;
	SA.storeColor = true;
	SA.useDepth = true;
	SA.showOnScreenIndex = 0;

	RenderPass* pass = new RenderPass();
	pass->addSubPass({ pipeline }, SA);
	pass->compile();

	FrameBuffer* frameBuffers = new FrameBuffer(s->size().width, s->size().height);
	pass->prepareOutputFrameBuffer(*frameBuffers);
	

	while (w.running()) {
		w.updateInput();
	
		SwapChainImage& image = s->AcquireImage();

		std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
		wait_semaphore_infos.push_back({
			image.getSemaphore(),                     // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT					// VkPipelineStageFlags   WaitingStage
			});

		commandBuffer.wait();
		commandBuffer.resetFence();
		commandBuffer.beginRecord();


		pass->setSwapChainImage(*frameBuffers, image);

		pass->draw(commandBuffer, *frameBuffers, vec2u({ 0,0 }), vec2u({ size.width, size.height }), { { 0.1f, 0.2f, 0.3f, 1.0f }, { 1.0f, 0 } });

		commandBuffer.endRecord();

		commandBuffer.submit(queue, wait_semaphore_infos, { commandBuffer.getSemaphore(0) });


		PresentInfo present_info = {
			s->getHandle(),                                    // VkSwapchainKHR         Swapchain
			image.getIndex()                              // uint32_t               ImageIndex
		};

		if (!PresentImage(presentQueue->getHandle(), { commandBuffer.getSemaphore(0) }, { present_info })) {
			continue;
		}
	}
	d->end();
}

```