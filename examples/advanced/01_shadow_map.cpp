/**
 * LavaCake Advanced Example 01: Shadow Mapping
 *
 * This example demonstrates:
 * - Two-pass rendering (shadow pass + scene pass)
 * - Creating and using a shadow map (depth texture)
 * - Rendering from light's perspective
 * - Sampling shadow map in fragment shader
 * - PCF (Percentage Closer Filtering) for soft shadows
 *
 * Scene: A floating cube above a plane, lit by a directional light from above.
 */

#include <LavaCake/GLFWSupport.hpp>
#include <LavaCake/CommandBuffer.hpp>
#include <LavaCake/GraphicPipeline.hpp>
#include <LavaCake/DynamicRendering.hpp>
#include <LavaCake/Buffer.hpp>
#include <LavaCake/UniformBuffer.hpp>
#include <LavaCake/DescriptorSet.hpp>
#include <LavaCake/DescriptorPool.hpp>
#include <LavaCake/Image.hpp>
#include <iostream>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::string root = PROJECT_ROOT;

using namespace LavaCake;

// Shadow map resolution
constexpr uint32_t SHADOW_MAP_SIZE = 2048;

// Uniform buffer for scene rendering
struct SceneUBO {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 lightSpaceMatrix;
    glm::vec3 lightPos;
    float padding1;
    glm::vec3 viewPos;
    float padding2;
};

// Uniform buffer for shadow pass
struct LightSpaceUBO {
    glm::mat4 lightSpaceMatrix;
};

// Generate cube geometry (positions, normals, colors, indices)
void generateCube(float size, const glm::vec3& offset, const glm::vec3& color,
                  std::vector<float>& positions, std::vector<float>& normals,
                  std::vector<float>& colors, std::vector<uint32_t>& indices) {
    uint32_t baseIndex = static_cast<uint32_t>(positions.size() / 3);
    float h = size / 2.0f;

    // 6 faces, 4 vertices each = 24 vertices for proper normals
    // Front face (z+)
    glm::vec3 frontNormal(0, 0, 1);
    positions.insert(positions.end(), {offset.x - h, offset.y - h, offset.z + h});
    positions.insert(positions.end(), {offset.x + h, offset.y - h, offset.z + h});
    positions.insert(positions.end(), {offset.x + h, offset.y + h, offset.z + h});
    positions.insert(positions.end(), {offset.x - h, offset.y + h, offset.z + h});
    for (int i = 0; i < 4; i++) { normals.insert(normals.end(), {frontNormal.x, frontNormal.y, frontNormal.z}); }
    for (int i = 0; i < 4; i++) { colors.insert(colors.end(), {color.r, color.g, color.b}); }
    indices.insert(indices.end(), {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3});
    baseIndex += 4;

    // Back face (z-)
    glm::vec3 backNormal(0, 0, -1);
    positions.insert(positions.end(), {offset.x + h, offset.y - h, offset.z - h});
    positions.insert(positions.end(), {offset.x - h, offset.y - h, offset.z - h});
    positions.insert(positions.end(), {offset.x - h, offset.y + h, offset.z - h});
    positions.insert(positions.end(), {offset.x + h, offset.y + h, offset.z - h});
    for (int i = 0; i < 4; i++) { normals.insert(normals.end(), {backNormal.x, backNormal.y, backNormal.z}); }
    for (int i = 0; i < 4; i++) { colors.insert(colors.end(), {color.r, color.g, color.b}); }
    indices.insert(indices.end(), {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3});
    baseIndex += 4;

    // Top face (y+)
    glm::vec3 topNormal(0, 1, 0);
    positions.insert(positions.end(), {offset.x - h, offset.y + h, offset.z + h});
    positions.insert(positions.end(), {offset.x + h, offset.y + h, offset.z + h});
    positions.insert(positions.end(), {offset.x + h, offset.y + h, offset.z - h});
    positions.insert(positions.end(), {offset.x - h, offset.y + h, offset.z - h});
    for (int i = 0; i < 4; i++) { normals.insert(normals.end(), {topNormal.x, topNormal.y, topNormal.z}); }
    for (int i = 0; i < 4; i++) { colors.insert(colors.end(), {color.r, color.g, color.b}); }
    indices.insert(indices.end(), {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3});
    baseIndex += 4;

    // Bottom face (y-)
    glm::vec3 bottomNormal(0, -1, 0);
    positions.insert(positions.end(), {offset.x - h, offset.y - h, offset.z - h});
    positions.insert(positions.end(), {offset.x + h, offset.y - h, offset.z - h});
    positions.insert(positions.end(), {offset.x + h, offset.y - h, offset.z + h});
    positions.insert(positions.end(), {offset.x - h, offset.y - h, offset.z + h});
    for (int i = 0; i < 4; i++) { normals.insert(normals.end(), {bottomNormal.x, bottomNormal.y, bottomNormal.z}); }
    for (int i = 0; i < 4; i++) { colors.insert(colors.end(), {color.r, color.g, color.b}); }
    indices.insert(indices.end(), {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3});
    baseIndex += 4;

    // Right face (x+)
    glm::vec3 rightNormal(1, 0, 0);
    positions.insert(positions.end(), {offset.x + h, offset.y - h, offset.z + h});
    positions.insert(positions.end(), {offset.x + h, offset.y - h, offset.z - h});
    positions.insert(positions.end(), {offset.x + h, offset.y + h, offset.z - h});
    positions.insert(positions.end(), {offset.x + h, offset.y + h, offset.z + h});
    for (int i = 0; i < 4; i++) { normals.insert(normals.end(), {rightNormal.x, rightNormal.y, rightNormal.z}); }
    for (int i = 0; i < 4; i++) { colors.insert(colors.end(), {color.r, color.g, color.b}); }
    indices.insert(indices.end(), {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3});
    baseIndex += 4;

    // Left face (x-)
    glm::vec3 leftNormal(-1, 0, 0);
    positions.insert(positions.end(), {offset.x - h, offset.y - h, offset.z - h});
    positions.insert(positions.end(), {offset.x - h, offset.y - h, offset.z + h});
    positions.insert(positions.end(), {offset.x - h, offset.y + h, offset.z + h});
    positions.insert(positions.end(), {offset.x - h, offset.y + h, offset.z - h});
    for (int i = 0; i < 4; i++) { normals.insert(normals.end(), {leftNormal.x, leftNormal.y, leftNormal.z}); }
    for (int i = 0; i < 4; i++) { colors.insert(colors.end(), {color.r, color.g, color.b}); }
    indices.insert(indices.end(), {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3});
}

// Generate plane geometry
void generatePlane(float size, float y, const glm::vec3& color,
                   std::vector<float>& positions, std::vector<float>& normals,
                   std::vector<float>& colors, std::vector<uint32_t>& indices) {
    uint32_t baseIndex = static_cast<uint32_t>(positions.size() / 3);
    float h = size / 2.0f;

    // Single quad facing up
    glm::vec3 normal(0, 1, 0);

    positions.insert(positions.end(), {-h, y, -h});
    positions.insert(positions.end(), { h, y, -h});
    positions.insert(positions.end(), { h, y,  h});
    positions.insert(positions.end(), {-h, y,  h});

    for (int i = 0; i < 4; i++) { normals.insert(normals.end(), {normal.x, normal.y, normal.z}); }
    for (int i = 0; i < 4; i++) { colors.insert(colors.end(), {color.r, color.g, color.b}); }

    // Counter-clockwise winding when viewed from above (normal points up)
    indices.insert(indices.end(), {baseIndex, baseIndex + 2, baseIndex + 1, baseIndex, baseIndex + 3, baseIndex + 2});
}

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Advanced 01 - Shadow Mapping", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device(surfaceConfig, 1);
    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Advanced Example 01: Shadow Mapping\n";

        // Generate scene geometry
        std::vector<float> positions, normals, colors;
        std::vector<uint32_t> indices;

        // Floating cube (orange)
        generateCube(1.0f, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.5f, 0.1f),
                     positions, normals, colors, indices);

        // Ground plane (gray)
        generatePlane(6.0f, -0.5f, glm::vec3(0.6f, 0.6f, 0.6f),
                      positions, normals, colors, indices);

        std::cout << "Scene geometry: " << positions.size() / 3 << " vertices, "
                  << indices.size() << " indices\n";

        // Create storage buffers
        LavaCake::Buffer positionBuffer(device, positions, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer normalBuffer(device, normals, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer colorBuffer(device, colors, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer indexBuffer(device, indices, vk::BufferUsageFlagBits::eStorageBuffer);

        // Create uniform buffers
        LavaCake::UniformBuffer sceneUBO(device);
        SceneUBO sceneData{};
        sceneUBO.addVariable("scene", sceneData);
        sceneUBO.end();

        LavaCake::UniformBuffer lightSpaceUBO(device);
        LightSpaceUBO lightData{};
        lightSpaceUBO.addVariable("light", lightData);
        lightSpaceUBO.end();

        // Create shadow map (depth texture)
        vk::Format shadowFormat = vk::Format::eD32Sfloat;
        LavaCake::Image shadowMapImage(
            device,
            SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1,
            shadowFormat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled
        );

        LavaCake::ImageView shadowMapView(
            shadowMapImage,
            vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eDepth
        );

        // Create shadow map sampler
        LavaCake::Sampler shadowSampler(
            device,
            vk::SamplerMipmapMode::eNearest,
            0.0f, 0.0f, 0.0f,
            false, 1.0f,
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerAddressMode::eClampToBorder,
            vk::SamplerAddressMode::eClampToBorder,
            vk::SamplerAddressMode::eClampToBorder
        );

        std::cout << "Shadow map created: " << SHADOW_MAP_SIZE << "x" << SHADOW_MAP_SIZE << "\n";

        // Create depth buffer for scene rendering
        vk::Extent2D extent = device.getSwapchainExtent();
        vk::Format depthFormat = vk::Format::eD32Sfloat;

        LavaCake::Image depthImage(
            device,
            extent.width, extent.height, 1,
            depthFormat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment
        );

        LavaCake::ImageView depthImageView(
            depthImage,
            vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eDepth
        );

        // === Shadow Pass Descriptor Set ===
        LavaCake::DescriptorSetLayout shadowDescLayout =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)  // positions
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)  // indices
                .addBinding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)  // lightSpace
                .build();

        LavaCake::DescriptorPool shadowDescPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(2)
                .addUniformBuffers(1)
                .setMaxSets(1)
                .build();

        vk::DescriptorSet shadowDescSet = shadowDescPool.allocate(shadowDescLayout);

        LavaCake::DescriptorSetUpdater(device, shadowDescSet)
            .bindStorageBuffer(0, positionBuffer)
            .bindStorageBuffer(1, indexBuffer)
            .bindUniformBuffer(2, lightSpaceUBO, 0, lightSpaceUBO.size())
            .update();

        // === Scene Pass Descriptor Sets ===
        // Set 0: Geometry and transforms
        LavaCake::DescriptorSetLayout sceneDescLayout0 =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addStorageBuffer(0, vk::ShaderStageFlagBits::eVertex)  // positions
                .addStorageBuffer(1, vk::ShaderStageFlagBits::eVertex)  // normals
                .addStorageBuffer(2, vk::ShaderStageFlagBits::eVertex)  // colors
                .addStorageBuffer(3, vk::ShaderStageFlagBits::eVertex)  // indices
                .addBinding(4, vk::DescriptorType::eUniformBuffer,
                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)  // transforms
                .build();

        // Set 1: Shadow map
        LavaCake::DescriptorSetLayout sceneDescLayout1 =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                .build();

        LavaCake::DescriptorPool sceneDescPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(4)
                .addUniformBuffers(1)
                .addCombinedImageSamplers(1)
                .setMaxSets(2)
                .build();

        vk::DescriptorSet sceneDescSet0 = sceneDescPool.allocate(sceneDescLayout0);
        vk::DescriptorSet sceneDescSet1 = sceneDescPool.allocate(sceneDescLayout1);

        LavaCake::DescriptorSetUpdater(device, sceneDescSet0)
            .bindStorageBuffer(0, positionBuffer)
            .bindStorageBuffer(1, normalBuffer)
            .bindStorageBuffer(2, colorBuffer)
            .bindStorageBuffer(3, indexBuffer)
            .bindUniformBuffer(4, sceneUBO, 0, sceneUBO.size())
            .update();

        LavaCake::DescriptorSetUpdater(device, sceneDescSet1)
            .bindImage(0, shadowMapView, shadowSampler,
                       vk::ImageLayout::eShaderReadOnlyOptimal)
            .update();

        // === Create Pipelines ===
        // Shadow pipeline (depth-only)
        LavaCake::GraphicsPipeline shadowPipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/shadow_depth.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/shadow_depth.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .setDepthAttachmentFormat(shadowFormat)
            .setDepthTest(true, true, vk::CompareOp::eLess)
            .setBindlessVertexInput()
            .addDescriptorSetLayout(shadowDescLayout)
            .setCullMode(vk::CullModeFlagBits::eFront)  // Front-face culling reduces shadow acne
            .build();

        // Scene pipeline
        LavaCake::GraphicsPipeline scenePipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/shadow_scene.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/shadow_scene.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .addColorAttachmentFormat(device.getSwapchainFormat())
            .setDepthAttachmentFormat(depthFormat)
            .setDepthTest(true, true, vk::CompareOp::eLess)
            .setBindlessVertexInput()
            .addDescriptorSetLayout(sceneDescLayout0)
            .addDescriptorSetLayout(sceneDescLayout1)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .build();

        std::cout << "Pipelines created\n";

        // Create semaphores
        size_t swapchainImageCount = device.getSwapChainImagesNumber();
        std::vector<vk::Semaphore> imageAvailableSemaphores(swapchainImageCount);
        std::vector<vk::Semaphore> renderFinishedSemaphores(swapchainImageCount);
        vk::SemaphoreCreateInfo semaphoreInfo;

        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
        }

        // Transition depth images
        {
            LavaCake::CommandBuffer cmd (device);
            cmd.begin();

            // Shadow map starts in shader read layout for the render loop transition logic
            shadowMapImage.transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);
            depthImage.transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

            cmd.end();
            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = cmd;
            device.getGraphicQueue(0).submit(submitInfo);
            device.getGraphicQueue(0).waitIdle();
        }

        float angle = 0.0f;
        uint32_t currentFrame = 0;

        // Light position
        glm::vec3 lightPos(2.0f, 5.0f, 2.0f);
        glm::vec3 viewPos(4.0f, 3.0f, 4.0f);

        std::cout << "Starting render loop...\n";

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            angle += 0.005f;

            // Update light position (orbiting)
            float lightRadius = 4.0f;
            lightPos = glm::vec3(
                lightRadius * std::cos(angle),
                5.0f,
                lightRadius * std::sin(angle)
            );

            // Compute light space matrix (orthographic projection from light's POV)
            float orthoSize = 5.0f;
            glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 20.0f);
            glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 lightSpaceMatrix = lightProjection * lightView;

            // Update light space UBO
            lightData.lightSpaceMatrix = lightSpaceMatrix;
            lightSpaceUBO.setVariable("light", lightData);

            // Update scene UBO
            sceneData.model = glm::mat4(1.0f);
            sceneData.view = glm::lookAt(viewPos, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
            sceneData.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
            sceneData.projection[1][1] *= -1;  // Flip Y for Vulkan
            sceneData.lightSpaceMatrix = lightSpaceMatrix;
            sceneData.lightPos = lightPos;
            sceneData.viewPos = viewPos;
            sceneUBO.setVariable("scene", sceneData);

            // Acquire swapchain image
            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // Update uniform buffers
            lightSpaceUBO.update(cmdBuffer);
            sceneUBO.update(cmdBuffer);

            // === Shadow Pass ===
            // Transition shadow map for rendering
            {
                shadowMapImage.transitionLayout(
                    cmdBuffer, 
                    vk::ImageLayout::eShaderReadOnlyOptimal, 
                    vk::ImageLayout::eDepthStencilAttachmentOptimal);

            }

            // Begin shadow pass rendering
            LavaCake::DynamicRenderingContext shadowContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(vk::Extent2D{SHADOW_MAP_SIZE, SHADOW_MAP_SIZE})
                .setDepthAttachment(shadowMapView, 1.0f)
                .begin(cmdBuffer);

            shadowContext.setViewport(cmdBuffer,
                                       static_cast<float>(SHADOW_MAP_SIZE),
                                       static_cast<float>(SHADOW_MAP_SIZE));
            shadowContext.setScissor(cmdBuffer, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

            shadowPipeline.bind(cmdBuffer);
            shadowPipeline.bindDescriptorSets(cmdBuffer, {shadowDescSet});
            
            
            shadowPipeline.draw(cmdBuffer, static_cast<uint32_t>(indices.size()));

            shadowContext.end(cmdBuffer);

            // Transition shadow map for sampling
            {
                shadowMapImage.transitionLayout(
                    cmdBuffer, 
                    vk::ImageLayout::eDepthStencilAttachmentOptimal, 
                    vk::ImageLayout::eShaderReadOnlyOptimal);
            }

            // === Scene Pass ===
            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            LavaCake::DynamicRenderingContext sceneContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage,
                    vk::ClearColorValue(std::array<float, 4>{0.2f, 0.3f, 0.4f, 1.0f})
                )
                .setDepthAttachment(depthImageView, 1.0f)
                .begin(cmdBuffer);

            sceneContext.setDefaultViewportScissor(cmdBuffer);

            scenePipeline.bind(cmdBuffer);
            scenePipeline.bindDescriptorSets(cmdBuffer, {{sceneDescSet0, sceneDescSet1}});
            
            scenePipeline.draw(cmdBuffer, static_cast<uint32_t>(indices.size()));

            sceneContext.end(cmdBuffer);

            swapchainImage.prepareForPresentBarrier(cmdBuffer);

            cmdBuffer.end();

            // Submit
            vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submitInfo;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
            submitInfo.pWaitDstStageMask = &waitStage;
            submitInfo.commandBufferCount = 1;
            vk::CommandBuffer rawCmd = cmdBuffer.getCommandBuffer();
            submitInfo.pCommandBuffers = cmdBuffer;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

            device.getGraphicQueue(0).submit(submitInfo, cmdBuffer.getFence());
            cmdBuffer.markSubmitted();

            device.presentImage(swapchainImage, {renderFinishedSemaphores[currentFrame]});

            currentFrame = (currentFrame + 1) % swapchainImageCount;
        }

        device.waitForAllCommands();

        for (size_t i = 0; i < swapchainImageCount; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        }
    }

    device.releaseDevice();

    std::cout << "Example completed successfully!\n";
    std::cout << "Shadow mapping with PCF soft shadows demonstrated.\n";

    return 0;
}
