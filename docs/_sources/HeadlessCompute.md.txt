# Headless Compute

In this document, we will see a simple example of how to use LavaCake in headless mode to execute a compute shader.

## Compute shader

In this example, we will try to use a compute shader to add to buffer together and store the result in a third one.
To do that, we will use the following compute shader : 

```cpp
#version 450

layout( local_size_x = 32) in;

layout( set = 0, binding = 0 , r32f) uniform imageBuffer A;
layout( set = 0, binding = 1 , r32f) uniform imageBuffer B;
layout( set = 0, binding = 2 , r32f) uniform imageBuffer C;

void main() {
  uint Lxid = gl_GlobalInvocationID.x;
  float a = imageLoad(A, int(Lxid) ).x;
  float b = imageLoad(B, int(Lxid) ).x;
  float c = a + b;

  imageStore(C,int(Lxid),vec4(c,0,0,0)); 
}
```

## Vulkan initialisation

To initialize Vulkan in LavaCake, you need to get a reference to the Device singleton provided by LavaCake and initialize it.\
This singleton holds references to Physical and Logical Devices, Queues, Surface, and Instances.

We can initialize the Device with the following lines:
```cpp
Device* device = Device::getDevice();
device->initDevices(0, 1);
```
This will initialize the Device with 0 graphic queue and 1 compute queue.

We will need a command buffer to execute the shader on the GPU; we can initialize it as follows: 
```cpp
CommandBuffer commandBuffer;
```

## Creating buffers

First, we need to prepare the data we want to fill them with : 
```cpp
std::vector<float> A(dataSize);
std::vector<float> B(dataSize);
for (int i = 0; i < dataSize; i++) {
  A[i] = i;
  B[i] = i * 2;
}
```

Then we can create the two input buffers as follows :
```cpp
Buffer ABuffer(computeQueue, commandBuffer, A, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
Buffer BBuffer(computeQueue, commandBuffer, B, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
```
This way, the two buffers are created using the compute queue, with the command buffer, as Texel buffers and initialized with the data we prepared. 

Finally, we can create the result buffer with :
```cpp
Buffer CBuffer(dataSize * sizeof(float), VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
```
This way, we have to create a buffer that has the same size as the two inputs, that is also a texel buffer and can also be read from.


## Preparing the compute pipeline

We can create a compute pipeline as follows: 
```cpp
 ComputePipeline sumPipeline;
```

Then we can load and bind the compute shader to the compute pipeline as follows:
```cpp
ComputeShaderModule sumShader(prefix + "Data/Shaders/ArraySum/sum.comp.spv");
sumPipeline.setComputeModule(sumShader);
```

To bind the buffers to the compute pipeline, we need to create a descriptor set.
We can create one as follows : 
```cpp
std::shared_ptr <DescriptorSet>  descriptorSet = std::make_shared<DescriptorSet>();
```

Now we can add the buffers as follows:
```cpp
descriptorSet->addTexelBuffer(ABuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0);
descriptorSet->addTexelBuffer(BBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 1);
descriptorSet->addTexelBuffer(CBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);
```

This way describes how these buffers will be bound.
The ABuffer will be bound at the binding 0 in the compute stage.
The BBuffer will be bound at the binding 1 in the compute stage.
The CBuffer will be bound at the binding 2 in the compute stage.

We can now link the descriptor set to our pipeline as follows:
```cpp
sumPipeline.setDescriptorSet(descriptorSet);
```

Finally, we need to compile our pipeline :
```cpp
sumPipeline.compile();
```

## Executing the compute pipeline

First, we need to put our command buffer in a recording state:
```cpp
commandBuffer.beginRecord();
```

Then we can add a compute command in the command buffer: 
```cpp
sumPipeline.compute(commandBuffer, dataSize / 32, 1, 1);
```

We can now put our command buffer out of its recording state:
```cpp
commandBuffer.endRecord();
```

Now we can submit the command buffer to the compute queue : 
```cpp
commandBuffer.submit(computeQueue, {}, {});
```

Then we need to wait for the command to be finished:

```cpp
commandBuffer.wait();
```

Finally we need to reset the fence in the command buffer:
```cpp
commandBuffer.resetFence();
```

## Reding back the data

To read back the data, we need to prepare a vector to hold our result:
```cpp
std::vector<float> C;
```

Then we can use the readback function from the CBuffer: 
```cpp
CBuffer.readBack(computeQueue, commandBuffer, C);
```

Finally, we can print the result : 
```cpp
for (auto c : C) {
    std::cout << c << " ";
}
std::cout << std::endl;
```

## Final code
```cpp
#include <LavaCake/Framework/Framework.h>
using namespace LavaCake::Framework;

#define dataSize 4096

int main() {
  Device* d = Device::getDevice();
  d->initDevices(1, 0);

  ComputeQueue computeQueue = d->getComputeQueue(0);
  CommandBuffer commandBuffer;

  std::vector<float> A(dataSize);
  std::vector<float> B(dataSize);
  for (int i = 0; i < dataSize; i++) {
    A[i] = i;
    B[i] = i * 2;
  }

  Buffer ABuffer(computeQueue, commandBuffer, A, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
  Buffer BBuffer(computeQueue, commandBuffer, B, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
  Buffer CBuffer(dataSize * sizeof(float), VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

  std::shared_ptr <DescriptorSet>  descriptorSet = std::make_shared<DescriptorSet>();

  descriptorSet->addTexelBuffer(ABuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0);
  descriptorSet->addTexelBuffer(BBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 1);
  descriptorSet->addTexelBuffer(CBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 2);

  ComputeShaderModule sumShader("Data/Shaders/ArraySum/sum.comp.spv");
  
  ComputePipeline sumPipeline;
  sumPipeline.setDescriptorSet(descriptorSet);

  sumPipeline.setComputeModule(sumShader);
  sumPipeline.compile();


  commandBuffer.beginRecord();
  sumPipeline.compute(commandBuffer, dataSize / 32, 1, 1);
  commandBuffer.endRecord();
  commandBuffer.submit(computeQueue, {}, {});
  commandBuffer.wait();
  commandBuffer.resetFence();

  std::vector<float> C;
  CBuffer.readBack(computeQueue, commandBuffer, C);

  for (auto c : C) {
    std::cout << c << " ";
  }
  std::cout << std::endl;

  return 0;
}
```
