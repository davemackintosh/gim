#pragma once

#include "gim/ecs/components/camera.hpp"
#include "gim/ecs/engine/entity_manager.hpp"
#include "vulkan/vulkan_core.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL_surface.h>
#include <VkBootstrap.h>
#include <algorithm>
#include <gim/ecs/components/engine-state.hpp>
#include <gim/ecs/components/triangle-shader.hpp>
#include <gim/ecs/engine/system_manager.hpp>
#include <gim/engine.hpp>
#include <gim/vulkan/instance.hpp>
#include <gim/vulkan/utils.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <utility>

namespace gim::ecs::systems {
const int MAX_FRAMES_IN_FLIGHT = 2;

auto VK_CHECK_RESULT(VkResult result, const std::string &message) -> VkResult {
    if ((result) != VK_SUCCESS) {
        throw std::runtime_error("failed to " + (message) + " (Result code: " +
                                 std::to_string(result) + ")");
    } else {
        std::cout << "Success: " + (message) << std::endl;
    }

    return result;
}

class VulkanRendererSystem : public gim::ecs::ISystem {
  private:
    // ECS.
    std::vector<Entity> entities;
    std::shared_ptr<ComponentManager> componentManager;

    // Vulkan.
    gim::vulkan::Instance instance;
    VkBuffer vertexBuffer;
    VkBuffer uniformBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkDeviceMemory uniformBufferMemory;
    VkDescriptorSet descriptorSet;
    VkDescriptorPool descriptorPool;

    bool readyToFinishInitialization = false;

    // Engine.
    std::shared_ptr<gim::ecs::components::Shader::TriangleShader> shader;
    std::shared_ptr<gim::ecs::components::Camera::Component> camera;

  public:
    VulkanRendererSystem() : instance(gim::vulkan::Instance()) {}
    ~VulkanRendererSystem() override {
        vkDestroyBuffer(instance.device.device, vertexBuffer, nullptr);
    }

#pragma mark - ECS

    auto getSignature() -> std::shared_ptr<Signature> override {
        auto signature = std::make_shared<Signature>();
        signature->set<gim::ecs::components::Camera::Component>();
        signature->set<gim::ecs::components::EngineState::Component>();
        signature->set<gim::ecs::components::Shader::TriangleShader>();

        return signature;
    }

    auto processInput(SDL_Event &event) -> void {
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_w)
                camera->position += camera->speed * camera->front;
            if (event.key.keysym.sym == SDLK_s)
                camera->position -= camera->speed * camera->front;
            if (event.key.keysym.sym == SDLK_a)
                camera->position -=
                    glm::normalize(glm::cross(camera->front, camera->up)) *
                    camera->speed;
            if (event.key.keysym.sym == SDLK_d)
                camera->position +=
                    glm::normalize(glm::cross(camera->front, camera->up)) *
                    camera->speed;
        }
    }

    auto handleMouseMotion(SDL_Event &event) -> void {
        // Update cameraYaw and cameraPitch based on mouse movement
        if (event.type == SDL_MOUSEMOTION) {
            float xoffset = event.motion.xrel * camera->sensitivity;
            float yoffset = -event.motion.yrel *
                            camera->sensitivity; // y-coordinates are reversed

            camera->yaw += xoffset;
            camera->pitch += yoffset;

            // Clamp pitch to prevent flipping
            if (camera->pitch > 89.0f)
                camera->pitch = 89.0f;
            if (camera->pitch < -89.0f)
                camera->pitch = -89.0f;

            glm::vec3 front;
            front.x = cos(glm::radians(camera->yaw)) *
                      cos(glm::radians(camera->pitch));
            front.y = sin(glm::radians(camera->pitch));
            front.z = sin(glm::radians(camera->yaw)) *
                      cos(glm::radians(camera->pitch));
            camera->front = glm::normalize(front);
        }
    }

    auto update() -> void override {
        auto engineStatePair = componentManager->getTComponentWithEntity<
            gim::ecs::components::EngineState::Component>(getEntities());
        auto triangleShaderPair = componentManager->getTComponentWithEntity<
            gim::ecs::components::Shader::TriangleShader>(getEntities());
        auto cameraPair = componentManager->getTComponentWithEntity<
            gim::ecs::components::Camera::Component>(getEntities());

        if (!engineStatePair.has_value()) {
            std::cout << "Engine state component not found!" << std::endl;
            return;
        }

        if (!triangleShaderPair.has_value()) {
            std::cout << "Triangle shader not found!" << std::endl;
            return;
        }

        if (!cameraPair.has_value()) {
            std::cout << "Camera not found!" << std::endl;
            return;
        }

        auto [_e, engineState] = engineStatePair.value();
        auto [_t, triangleShaderComponent] = triangleShaderPair.value();
        auto [_c, cameraComponent] = cameraPair.value();

        shader = triangleShaderComponent;
        camera = cameraComponent;

        if (!readyToFinishInitialization) {
            readyToFinishInitialization = true;

            finishCreatingGraphicsPipeline();
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                engineState->state =
                    gim::ecs::components::EngineState::Quitting;
                return;
            } else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_RESIZED) {
            }

            processInput(event);
            handleMouseMotion(event);
        }

        updateUniformBuffer();
        drawFrame();
    }

    auto insertEntity(Entity entity) -> void override {
        entities.push_back(entity);
    }
    auto removeEntity(Entity entity) -> void override {
        entities.erase(std::remove(entities.begin(), entities.end(), entity),
                       entities.end());
    }
    auto getEntities() -> std::vector<Entity> const & override {
        return entities;
    }
    auto setComponentManager(std::shared_ptr<ComponentManager> cm)
        -> void override {
        componentManager = cm;
    }
    auto getComponentManager() -> std::shared_ptr<ComponentManager> override {
        return componentManager;
    }

#pragma mark - Vulkan pipeline creation.

    auto finishCreatingGraphicsPipeline() -> void {
        createGraphicsPipeline();
        createFramebuffers();
        createVertexBuffer();
        createUniformBuffer();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
    }

    [[nodiscard]] auto findMemoryType(uint32_t typeFilter,
                                      VkMemoryPropertyFlags properties) const
        -> uint32_t {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(instance.device.physical_device,
                                            &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) ==
                    properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    auto createVertexBuffer() -> void {
        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = shader->getBindings()->getBufferSize(),
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VK_CHECK_RESULT(vkCreateBuffer(instance.device.device, &bufferInfo,
                                       nullptr, &vertexBuffer),
                        "create vertex buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(instance.device.device, vertexBuffer,
                                      &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            findMemoryType(memRequirements.memoryTypeBits,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(instance.device.device, &allocInfo,
                                         nullptr, &vertexBufferMemory),
                        "allocate vertex buffer memory!");

        vkBindBufferMemory(instance.device.device, vertexBuffer,
                           vertexBufferMemory, 0);

        void *data;
        VK_CHECK_RESULT(vkMapMemory(instance.device.device, vertexBufferMemory,
                                    0, bufferInfo.size, 0, &data),
                        "map vertex buffer memory.");
        memcpy(data, shader->getBindings()->vertData->vertices.data(),
               (size_t)bufferInfo.size);
        vkUnmapMemory(instance.device.device, vertexBufferMemory);
    }

    auto updateUniformBuffer() -> void {
        auto ubo = camera->getShaderUBO().get();
        void *data;
        VK_CHECK_RESULT(vkMapMemory(instance.device, uniformBufferMemory, 0,
                                    camera->getBufferSize(), 0, &data),
                        "map update uniform buffer memory.");
        memcpy(data, &ubo, camera->getBufferSize());
        vkUnmapMemory(instance.device, uniformBufferMemory);
    }

    auto createUniformBuffer() -> void {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = camera->getBufferSize();
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK_RESULT(vkCreateBuffer(instance.device.device, &bufferInfo,
                                       nullptr, &uniformBuffer),
                        "create vertex buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(instance.device.device, uniformBuffer,
                                      &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            findMemoryType(memRequirements.memoryTypeBits,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(instance.device.device, &allocInfo,
                                         nullptr, &uniformBufferMemory),
                        "allocate uniform buffer memory!");

        vkBindBufferMemory(instance.device.device, uniformBuffer,
                           uniformBufferMemory, 0);

        void *data;
        VK_CHECK_RESULT(vkMapMemory(instance.device.device, uniformBufferMemory,
                                    0, bufferInfo.size, 0, &data),
                        "bind uniform buffer memory");
        memcpy(data, camera->getShaderUBO().get(), (size_t)bufferInfo.size);
        vkUnmapMemory(instance.device.device, vertexBufferMemory);
    }

    auto createGraphicsPipeline() -> void {
        if (!shader) {
            std::cout << "Required triangle shader not found!" << std::endl;
            return;
        }
        auto shader_stages = shader->getShaderStageCreateInfo(instance.device);

        auto shaderBindings = shader->getBindings();
        auto shaderVertexBindings =
            shaderBindings->getVertexBindingDescriptionsForDevice();
        auto shaderAttributes =
            shaderBindings->getVertexAttributesDescriptionsForDevice();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount =
            static_cast<uint32_t>(shaderVertexBindings.size());
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(shaderAttributes.size());

        vertexInputInfo.pVertexBindingDescriptions =
            shaderVertexBindings.data();
        vertexInputInfo.pVertexAttributeDescriptions = shaderAttributes.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
        input_assembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)instance.swapchain.extent.width;
        viewport.height = (float)instance.swapchain.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = instance.swapchain.extent,
        };

        VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
        };

        VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &colorBlendAttachment;

        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;

        VkDescriptorSetLayoutBinding layoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &layoutBinding,
        };

        VkDescriptorSetLayout descriptorSetLayout;
        vkCreateDescriptorSetLayout(instance.device, &layoutInfo, nullptr,
                                    &descriptorSetLayout);

        VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1, // Adjust as needed
        };

        VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1, // Adjust as needed
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize,
        };

        VK_CHECK_RESULT(vkCreateDescriptorPool(instance.device, &poolInfo,
                                               nullptr, &descriptorPool),
                        "Failed to create descriptor pool!");

        VkDescriptorSetLayout layouts[] = {descriptorSetLayout};
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = layouts,
        };

        VK_CHECK_RESULT(vkAllocateDescriptorSets(instance.device, &allocInfo,
                                                 &descriptorSet),
                        "allocate descriptor set!");

        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffer,
            .offset = 0,
            .range = camera->getBufferSize(),
        };

        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        };

        vkUpdateDescriptorSets(instance.device, 1, &descriptorWrite, 0,
                               nullptr);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &descriptorSetLayout,
        };

        VK_CHECK_RESULT(vkCreatePipelineLayout(instance.device,
                                               &pipelineLayoutInfo, nullptr,
                                               &instance.data.pipeline_layout),
                        "create pipeline layout!");

        std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                      VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data(),
        };

        VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &color_blending,
            .pDynamicState = &dynamicInfo,
            .layout = instance.data.pipeline_layout,
            .renderPass = instance.data.render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
        };

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(
                            instance.device, VK_NULL_HANDLE, 1, &pipelineInfo,
                            nullptr, &instance.data.graphics_pipeline),
                        "create graphics pipeline!");
    }

    auto createFramebuffers() -> void {
        instance.data.swapchain_images =
            instance.swapchain.get_images().value();
        instance.data.swapchain_image_views =
            instance.swapchain.get_image_views().value();

        instance.data.framebuffers.resize(
            instance.data.swapchain_image_views.size());

        for (size_t i = 0; i < instance.data.swapchain_image_views.size();
             i++) {
            VkImageView attachments[] = {
                instance.data.swapchain_image_views[i]};

            VkFramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = instance.data.render_pass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments;
            framebuffer_info.width = instance.swapchain.extent.width;
            framebuffer_info.height = instance.swapchain.extent.height;
            framebuffer_info.layers = 1;

            VK_CHECK_RESULT(vkCreateFramebuffer(instance.device,
                                                &framebuffer_info, nullptr,
                                                &instance.data.framebuffers[i]),
                            "create framebuffer!");
        }
    }

    auto createCommandPool() -> void {
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex =
            instance.device.get_queue_index(vkb::QueueType::graphics).value();

        VK_CHECK_RESULT(vkCreateCommandPool(instance.device, &pool_info,
                                            nullptr,
                                            &instance.data.command_pool),
                        "create command pool!");
    }

    auto createCommandBuffers() -> void {
        instance.data.command_buffers.resize(instance.data.framebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = instance.data.command_pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount =
            (uint32_t)instance.data.command_buffers.size();

        VK_CHECK_RESULT(
            vkAllocateCommandBuffers(instance.device, &allocInfo,
                                     instance.data.command_buffers.data()),
            "allocate command buffers!");

        for (size_t i = 0; i < instance.data.command_buffers.size(); i++) {
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            VK_CHECK_RESULT(vkBeginCommandBuffer(
                                instance.data.command_buffers[i], &begin_info),
                            "begin recording command buffers!");

            VkRenderPassBeginInfo render_pass_info = {};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.renderPass = instance.data.render_pass;
            render_pass_info.framebuffer = instance.data.framebuffers[i];
            render_pass_info.renderArea.offset = {0, 0};
            render_pass_info.renderArea.extent = instance.swapchain.extent;
            VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
            render_pass_info.clearValueCount = 1;
            render_pass_info.pClearValues = &clearColor;

            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)instance.swapchain.extent.width;
            viewport.height = (float)instance.swapchain.extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor = {};
            scissor.offset = {0, 0};
            scissor.extent = instance.swapchain.extent;

            vkCmdSetViewport(instance.data.command_buffers[i], 0, 1, &viewport);
            vkCmdSetScissor(instance.data.command_buffers[i], 0, 1, &scissor);
            vkCmdBeginRenderPass(instance.data.command_buffers[i],
                                 &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(instance.data.command_buffers[i],
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              instance.data.graphics_pipeline);
            VkBuffer vertexBuffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            // TODO: Will probably need to total up the bindings by counting
            // the vertex buffers.
            vkCmdBindVertexBuffers(instance.data.command_buffers[i], 0, 1,
                                   vertexBuffers, offsets);
            vkCmdDraw(instance.data.command_buffers[i], 3, 1, 0, 0);
            vkCmdEndRenderPass(instance.data.command_buffers[i]);

            VK_CHECK_RESULT(
                vkEndCommandBuffer(instance.data.command_buffers[i]),
                "record command buffer!");
        }
    }

    auto createSyncObjects() -> void {
        instance.data.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        instance.data.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
        instance.data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
        instance.data.image_in_flight.resize(instance.swapchain.image_count,
                                             VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_CHECK_RESULT(
                vkCreateSemaphore(instance.device, &semaphore_info, nullptr,
                                  &instance.data.available_semaphores[i]),
                "create sync objects!");
            VK_CHECK_RESULT(
                vkCreateSemaphore(instance.device, &semaphore_info, nullptr,
                                  &instance.data.finished_semaphore[i]),
                "create sync objects!");
            VK_CHECK_RESULT(vkCreateFence(instance.device, &fence_info, nullptr,
                                          &instance.data.in_flight_fences[i]),
                            "create sync objects!");
        }
    }

    auto recreate_swapchain() -> auto {
        vkDeviceWaitIdle(instance.device);
        vkDestroyCommandPool(instance.device, instance.data.command_pool,
                             nullptr);

        for (auto framebuffer : instance.data.framebuffers) {
            vkDestroyFramebuffer(instance.device, framebuffer, nullptr);
        }

        instance.swapchain.destroy_image_views(
            instance.data.swapchain_image_views);

        instance.createSwapchain();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
    }

    auto drawFrame() -> void {
        vkWaitForFences(
            instance.device, 1,
            &instance.data.in_flight_fences[instance.data.current_frame],
            VK_TRUE, UINT64_MAX);

        uint32_t image_index = 0;
        VkResult result = vkAcquireNextImageKHR(
            instance.device, instance.swapchain, UINT64_MAX,
            instance.data.available_semaphores[instance.data.current_frame],
            VK_NULL_HANDLE, &image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return recreate_swapchain();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swapchain image!");
        }

        if (instance.data.image_in_flight[image_index] != VK_NULL_HANDLE) {
            vkWaitForFences(instance.device, 1,
                            &instance.data.image_in_flight[image_index],
                            VK_TRUE, UINT64_MAX);
        }
        instance.data.image_in_flight[image_index] =
            instance.data.in_flight_fences[instance.data.current_frame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {
            instance.data.available_semaphores[instance.data.current_frame]};
        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = wait_semaphores;
        submitInfo.pWaitDstStageMask = wait_stages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers =
            &instance.data.command_buffers[image_index];

        VkSemaphore signal_semaphores[] = {
            instance.data.finished_semaphore[instance.data.current_frame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signal_semaphores;

        vkResetFences(
            instance.device, 1,
            &instance.data.in_flight_fences[instance.data.current_frame]);

        VK_CHECK_RESULT(
            vkQueueSubmit(
                instance.data.graphics_queue, 1, &submitInfo,
                instance.data.in_flight_fences[instance.data.current_frame]),
            "submit draw command buffer\n");

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapChains[] = {instance.swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapChains;

        present_info.pImageIndices = &image_index;

        result = vkQueuePresentKHR(instance.data.present_queue, &present_info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            return recreate_swapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swapchain image!");
        }

        instance.data.current_frame =
            (instance.data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};
} // namespace gim::ecs::systems
