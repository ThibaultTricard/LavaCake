/**
 * LavaCake Advanced Example 02: Multiple Point Lights with Bindless Shadow Maps
 *
 * This example demonstrates:
 * - Multiple point lights with individual shadow maps
 * - Bindless texture arrays for shadow map sampling
 * - Push constants for efficient per-light rendering
 * - Dynamic descriptor indexing in shaders
 *
 * Scene: Multiple colored point lights casting shadows on objects.
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

// Configuration
constexpr uint32_t SHADOW_MAP_SIZE = 2048;
constexpr uint32_t MAX_LIGHTS = 4;
constexpr uint32_t NUM_LIGHTS = 4;

// Light structure (must match shader)
struct Light {
    glm::vec4 position;      // xyz = position, w = intensity
    glm::vec4 color;         // rgb = color, a = radius
    glm::mat4 lightSpaceMatrix;
};

// Scene uniform buffer structure
struct SceneUBO {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 viewPos;
    uint32_t numLights;
    Light lights[MAX_LIGHTS];
};

// Push constant for shadow pass
struct ShadowPushConstant {
    glm::mat4 lightSpaceMatrix;
    uint32_t lightIndex;
};

// Generate cube geometry
void generateCube(float size, const glm::vec3& offset, const glm::vec3& color,
                  std::vector<float>& positions, std::vector<float>& normals,
                  std::vector<float>& colors, std::vector<uint32_t>& indices) {
    uint32_t baseIndex = static_cast<uint32_t>(positions.size() / 3);
    float h = size / 2.0f;

    // Face data: normal, then 4 vertices
    struct Face {
        glm::vec3 normal;
        glm::vec3 verts[4];
    };

    Face faces[] = {
        // Front (+Z)
        {{0, 0, 1}, {{-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}}},
        // Back (-Z)
        {{0, 0, -1}, {{h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}}},
        // Top (+Y)
        {{0, 1, 0}, {{-h, h, h}, {h, h, h}, {h, h, -h}, {-h, h, -h}}},
        // Bottom (-Y)
        {{0, -1, 0}, {{-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h}}},
        // Right (+X)
        {{1, 0, 0}, {{h, -h, h}, {h, -h, -h}, {h, h, -h}, {h, h, h}}},
        // Left (-X)
        {{-1, 0, 0}, {{-h, -h, -h}, {-h, -h, h}, {-h, h, h}, {-h, h, -h}}}
    };

    for (const auto& face : faces) {
        for (int i = 0; i < 4; i++) {
            positions.push_back(face.verts[i].x + offset.x);
            positions.push_back(face.verts[i].y + offset.y);
            positions.push_back(face.verts[i].z + offset.z);
            normals.push_back(face.normal.x);
            normals.push_back(face.normal.y);
            normals.push_back(face.normal.z);
            colors.push_back(color.r);
            colors.push_back(color.g);
            colors.push_back(color.b);
        }
        indices.insert(indices.end(), {
            baseIndex, baseIndex + 1, baseIndex + 2,
            baseIndex, baseIndex + 2, baseIndex + 3
        });
        baseIndex += 4;
    }
}

// Generate plane geometry
void generatePlane(float size, float y, const glm::vec3& color,
                   std::vector<float>& positions, std::vector<float>& normals,
                   std::vector<float>& colors, std::vector<uint32_t>& indices) {
    uint32_t baseIndex = static_cast<uint32_t>(positions.size() / 3);
    float h = size / 2.0f;

    glm::vec3 normal(0, 1, 0);
    glm::vec3 verts[] = {{-h, y, -h}, {h, y, -h}, {h, y, h}, {-h, y, h}};

    for (const auto& v : verts) {
        positions.insert(positions.end(), {v.x, v.y, v.z});
        normals.insert(normals.end(), {normal.x, normal.y, normal.z});
        colors.insert(colors.end(), {color.r, color.g, color.b});
    }

    // Counter-clockwise winding when viewed from above
    indices.insert(indices.end(), {baseIndex, baseIndex + 2, baseIndex + 1,
                                   baseIndex, baseIndex + 3, baseIndex + 2});
}

// Generate pillar (tall cube)
void generatePillar(const glm::vec3& position, float width, float height, const glm::vec3& color,
                    std::vector<float>& positions, std::vector<float>& normals,
                    std::vector<float>& colors, std::vector<uint32_t>& indices) {
    glm::vec3 offset = position + glm::vec3(0, height / 2.0f, 0);
    uint32_t baseIndex = static_cast<uint32_t>(positions.size() / 3);
    float hw = width / 2.0f;
    float hh = height / 2.0f;

    struct Face {
        glm::vec3 normal;
        glm::vec3 verts[4];
    };

    Face faces[] = {
        {{0, 0, 1}, {{-hw, -hh, hw}, {hw, -hh, hw}, {hw, hh, hw}, {-hw, hh, hw}}},
        {{0, 0, -1}, {{hw, -hh, -hw}, {-hw, -hh, -hw}, {-hw, hh, -hw}, {hw, hh, -hw}}},
        {{0, 1, 0}, {{-hw, hh, hw}, {hw, hh, hw}, {hw, hh, -hw}, {-hw, hh, -hw}}},
        {{0, -1, 0}, {{-hw, -hh, -hw}, {hw, -hh, -hw}, {hw, -hh, hw}, {-hw, -hh, hw}}},
        {{1, 0, 0}, {{hw, -hh, hw}, {hw, -hh, -hw}, {hw, hh, -hw}, {hw, hh, hw}}},
        {{-1, 0, 0}, {{-hw, -hh, -hw}, {-hw, -hh, hw}, {-hw, hh, hw}, {-hw, hh, -hw}}}
    };

    for (const auto& face : faces) {
        for (int i = 0; i < 4; i++) {
            positions.push_back(face.verts[i].x + offset.x);
            positions.push_back(face.verts[i].y + offset.y);
            positions.push_back(face.verts[i].z + offset.z);
            normals.push_back(face.normal.x);
            normals.push_back(face.normal.y);
            normals.push_back(face.normal.z);
            colors.push_back(color.r);
            colors.push_back(color.g);
            colors.push_back(color.b);
        }
        indices.insert(indices.end(), {
            baseIndex, baseIndex + 1, baseIndex + 2,
            baseIndex, baseIndex + 2, baseIndex + 3
        });
        baseIndex += 4;
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Advanced 02 - Multiple Lights (Bindless)", nullptr, nullptr);

    auto surfaceConfig = LavaCake::GLFW::createSurfaceConfig(window);
    LavaCake::Device device = LavaCake::createAdvancedDevice(surfaceConfig,1);
    {
        LavaCake::CommandBuffer cmdBuffer(device, true);

        std::cout << "LavaCake Advanced Example 02: Multiple Point Lights with Bindless Shadow Maps\n";

        // Generate scene geometry
        std::vector<float> positions, normals, colors;
        std::vector<uint32_t> indices;

        // Ground plane (white/gray)
        generatePlane(12.0f, 0.0f, glm::vec3(0.8f, 0.8f, 0.8f),
                      positions, normals, colors, indices);

        // Central cube (white)
        generateCube(1.5f, glm::vec3(0.0f, 0.75f, 0.0f), glm::vec3(0.9f, 0.9f, 0.9f),
                     positions, normals, colors, indices);

        // Pillars at corners
        generatePillar(glm::vec3(-3.0f, 0.0f, -3.0f), 0.5f, 2.5f, glm::vec3(0.7f, 0.7f, 0.7f),
                       positions, normals, colors, indices);
        generatePillar(glm::vec3(3.0f, 0.0f, -3.0f), 0.5f, 2.5f, glm::vec3(0.7f, 0.7f, 0.7f),
                       positions, normals, colors, indices);
        generatePillar(glm::vec3(-3.0f, 0.0f, 3.0f), 0.5f, 2.5f, glm::vec3(0.7f, 0.7f, 0.7f),
                       positions, normals, colors, indices);
        generatePillar(glm::vec3(3.0f, 0.0f, 3.0f), 0.5f, 2.5f, glm::vec3(0.7f, 0.7f, 0.7f),
                       positions, normals, colors, indices);

        // Small floating cubes
        generateCube(0.5f, glm::vec3(-2.0f, 1.0f, 0.0f), glm::vec3(0.9f, 0.9f, 0.9f),
                     positions, normals, colors, indices);
        generateCube(0.5f, glm::vec3(2.0f, 1.0f, 0.0f), glm::vec3(0.9f, 0.9f, 0.9f),
                     positions, normals, colors, indices);
        generateCube(0.5f, glm::vec3(0.0f, 1.0f, -2.0f), glm::vec3(0.9f, 0.9f, 0.9f),
                     positions, normals, colors, indices);
        generateCube(0.5f, glm::vec3(0.0f, 1.0f, 2.0f), glm::vec3(0.9f, 0.9f, 0.9f),
                     positions, normals, colors, indices);

        std::cout << "Scene geometry: " << positions.size() / 3 << " vertices, "
                  << indices.size() << " indices\n";

        // Create storage buffers
        LavaCake::Buffer positionBuffer(device, positions, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer normalBuffer(device, normals, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer colorBuffer(device, colors, vk::BufferUsageFlagBits::eStorageBuffer);
        LavaCake::Buffer indexBuffer(device, indices, vk::BufferUsageFlagBits::eStorageBuffer);

        // Create scene uniform buffer
        LavaCake::UniformBuffer sceneUBO(device);
        SceneUBO sceneData{};
        sceneUBO.addVariable("scene", sceneData);
        sceneUBO.end();

        // Create shadow maps for each light
        vk::Format shadowFormat = vk::Format::eD32Sfloat;
        std::vector<LavaCake::Image> shadowMapImages;
        shadowMapImages.reserve(NUM_LIGHTS);
        for (uint32_t i = 0; i < NUM_LIGHTS; i++) {
            shadowMapImages.emplace_back(
                device,
                SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1,
                shadowFormat,
                vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled
            );
        }

        std::vector<LavaCake::ImageView> shadowMapViews;
        shadowMapViews.reserve(NUM_LIGHTS);
        for (uint32_t i = 0; i < NUM_LIGHTS; i++) {
            shadowMapViews.emplace_back(
                shadowMapImages[i],
                vk::ImageViewType::e2D,
                vk::ImageAspectFlagBits::eDepth
            );
        }

        // Create shadow map sampler (shared by all shadow maps)
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

        std::cout << "Created " << NUM_LIGHTS << " shadow maps (" << SHADOW_MAP_SIZE << "x" << SHADOW_MAP_SIZE << ")\n";

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
                .build();

        LavaCake::DescriptorPool shadowDescPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(2)
                .setMaxSets(1)
                .build();

        vk::DescriptorSet shadowDescSet = shadowDescPool.allocate(shadowDescLayout);

        LavaCake::DescriptorSetUpdater(device, shadowDescSet)
            .bindStorageBuffer(0, positionBuffer)
            .bindStorageBuffer(1, indexBuffer)
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
                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
                .build();

        // Set 1: Bindless shadow maps array
        LavaCake::DescriptorSetLayout sceneDescLayout1 =
            LavaCake::DescriptorSetLayout::Builder(device)
                .addCombinedImageSampler(0, vk::ShaderStageFlagBits::eFragment, NUM_LIGHTS)
                .setBindless(NUM_LIGHTS)
                .build();

        LavaCake::DescriptorPool sceneDescPool =
            LavaCake::DescriptorPool::Builder(device)
                .addStorageBuffers(4)
                .addUniformBuffers(1)
                .addCombinedImageSamplers(NUM_LIGHTS)
                .setMaxSets(2)
                .setUpdateAfterBindFlag()
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

        // Bind all shadow maps as bindless texture array
        LavaCake::DescriptorSetUpdater(device, sceneDescSet1)
            .bindImageArray(0, shadowMapViews, shadowSampler,
                           vk::ImageLayout::eShaderReadOnlyOptimal)
            .update();

        // === Create Pipelines ===
        // Shadow pipeline with push constants
        LavaCake::GraphicsPipeline shadowPipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/multi_light_depth.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/multi_light_depth.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .setDepthAttachmentFormat(shadowFormat)
            .setDepthTest(true, true, vk::CompareOp::eLess)
            .setBindlessVertexInput()
            .addDescriptorSetLayout(shadowDescLayout)
            .addPushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShadowPushConstant))
            .setCullMode(vk::CullModeFlagBits::eNone)
            .build();

        // Scene pipeline
        LavaCake::GraphicsPipeline scenePipeline = LavaCake::GraphicsPipeline::Builder(device)
            .addShaderFromFile(root + "shaders/multi_light_scene.vert", vk::ShaderStageFlagBits::eVertex, LavaCake::ShadingLanguage::eGLSL)
            .addShaderFromFile(root + "shaders/multi_light_scene.frag", vk::ShaderStageFlagBits::eFragment, LavaCake::ShadingLanguage::eGLSL)
            .addColorAttachmentFormat(device.getSwapchainFormat())
            .setDepthAttachmentFormat(depthFormat)
            .setDepthTest(true, true, vk::CompareOp::eLess)
            .setBindlessVertexInput()
            .addDescriptorSetLayout(sceneDescLayout0)
            .addDescriptorSetLayout(sceneDescLayout1)
            .setCullMode(vk::CullModeFlagBits::eNone)
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
            LavaCake::CommandBuffer cmd(device);
            cmd.begin();

            // Transition all shadow maps to shader-read layout initially
            for (auto& shadowImage : shadowMapImages) {
                shadowImage.transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);
            }
            depthImage.transitionLayout(cmd, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

            cmd.end();
            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = cmd;
            device.getGraphicQueue(0).submit(submitInfo);
            device.getGraphicQueue(0).waitIdle();
        }

        float time = 0.0f;
        uint32_t currentFrame = 0;

        // Light colors
        glm::vec3 lightColors[NUM_LIGHTS] = {
            glm::vec3(1.0f, 0.3f, 0.3f),  // Red
            glm::vec3(0.3f, 1.0f, 0.3f),  // Green
            glm::vec3(0.3f, 0.3f, 1.0f),  // Blue
            glm::vec3(1.0f, 1.0f, 0.3f)   // Yellow
        };

        glm::vec3 viewPos(6.0f, 5.0f, 6.0f);

        std::cout << "Starting render loop with " << NUM_LIGHTS << " dynamic lights...\n";

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            cmdBuffer.waitForCompletion();
            cmdBuffer.reset();

            time += 0.01f;

            // Update light positions (orbiting at different heights and speeds)
            sceneData.numLights = NUM_LIGHTS;
            for (uint32_t i = 0; i < NUM_LIGHTS; i++) {
                float angle = time * (0.5f + i * 0.2f) + i * (glm::pi<float>() / 2.0f);
                float radius = 4.0f;
                float height = 4.0f;

                sceneData.lights[i].position = glm::vec4(
                    radius * std::cos(angle),
                    height,
                    radius * std::sin(angle),
                    2.0f  // intensity
                );
                sceneData.lights[i].color = glm::vec4(lightColors[i], 15.0f);  // radius

                // Compute light space matrix for shadow mapping
                glm::vec3 lightPos = glm::vec3(sceneData.lights[i].position);
                glm::mat4 lightProjection = glm::perspective(glm::radians(140.0f), 1.0f, 1.0f, 25.0f);
                lightProjection[1][1] *= -1;  // Flip Y for Vulkan
                glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                sceneData.lights[i].lightSpaceMatrix = lightProjection * lightView;
            }

            // Update camera
            sceneData.model = glm::mat4(1.0f);
            sceneData.view = glm::lookAt(viewPos, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
            sceneData.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
            sceneData.projection[1][1] *= -1;  // Flip Y for Vulkan
            sceneData.viewPos = viewPos;
            sceneUBO.setVariable("scene", sceneData);

            // Acquire swapchain image
            LavaCake::SwapChainImage& swapchainImage = device.aquireSwapChainImage(imageAvailableSemaphores[currentFrame]);

            cmdBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            // Update uniform buffer
            sceneUBO.update(cmdBuffer);

            // === Shadow Passes (one per light) ===
            for (uint32_t lightIdx = 0; lightIdx < NUM_LIGHTS; lightIdx++) {
                // Transition shadow map for rendering
                shadowMapImages[lightIdx].transitionLayout(
                    cmdBuffer,
                    vk::ImageLayout::eShaderReadOnlyOptimal,
                    vk::ImageLayout::eDepthStencilAttachmentOptimal
                );

                // Begin shadow pass rendering
                LavaCake::DynamicRenderingContext shadowContext = LavaCake::DynamicRenderingContext::Builder()
                    .setRenderArea(vk::Extent2D{SHADOW_MAP_SIZE, SHADOW_MAP_SIZE})
                    .setDepthAttachment(shadowMapViews[lightIdx], 1.0f)
                    .begin(cmdBuffer);

                shadowContext.setViewport(cmdBuffer,
                                          static_cast<float>(SHADOW_MAP_SIZE),
                                          static_cast<float>(SHADOW_MAP_SIZE));
                shadowContext.setScissor(cmdBuffer, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

                shadowPipeline.bind(cmdBuffer);

                shadowPipeline.bindDescriptorSets(cmdBuffer, {shadowDescSet});
              
                // Push light space matrix
                ShadowPushConstant pushData;
                pushData.lightSpaceMatrix = sceneData.lights[lightIdx].lightSpaceMatrix;
                pushData.lightIndex = lightIdx;

                shadowPipeline.pushConstants<ShadowPushConstant>(cmdBuffer,vk::ShaderStageFlagBits::eVertex,  0, pushData);

                shadowPipeline.draw(cmdBuffer, static_cast<uint32_t>(indices.size()));

                shadowContext.end(cmdBuffer);

                // Transition shadow map for sampling
                shadowMapImages[lightIdx].transitionLayout(
                    cmdBuffer,
                    vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    vk::ImageLayout::eShaderReadOnlyOptimal
                );
            }

            // === Scene Pass ===
            swapchainImage.prepareForAttachementBarrier(cmdBuffer);

            LavaCake::DynamicRenderingContext sceneContext = LavaCake::DynamicRenderingContext::Builder()
                .setRenderArea(device.getSwapchainExtent())
                .addColorAttachment(
                    swapchainImage,
                    vk::ClearColorValue(std::array<float, 4>{0.02f, 0.02f, 0.05f, 1.0f})
                )
                .setDepthAttachment(depthImageView, 1.0f)
                .begin(cmdBuffer);

            sceneContext.setDefaultViewportScissor(cmdBuffer);

            scenePipeline.bind(cmdBuffer);
            scenePipeline.bindDescriptorSets(cmdBuffer,{sceneDescSet0, sceneDescSet1} );
           
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
    std::cout << "Demonstrated: " << NUM_LIGHTS << " dynamic point lights with bindless shadow maps.\n";

    return 0;
}
