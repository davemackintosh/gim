#include <gim/ecs/systems/vulkan.hpp>

namespace gim::ecs::systems {
VulkanRendererSystem::VulkanRendererSystem() {
    instance = gim::vulkan::Instance();
}

VulkanRendererSystem::~VulkanRendererSystem() { instance.destroy(); }

auto VulkanRendererSystem::getSignature() -> std::shared_ptr<Signature> {
    auto signature = std::make_shared<Signature>();
    signature->set<gim::ecs::components::Camera::Component>();
    signature->set<gim::ecs::components::EngineState::Component>();
    signature->set<gim::ecs::components::Shader::ShaderBuilder>();

    return signature;
}

auto VulkanRendererSystem::processInput(SDL_Event &event) -> void {
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

auto VulkanRendererSystem::handleMouseMotion(SDL_Event &event) -> void {
    // Update cameraYaw and cameraPitch based on mouse movement
    if (event.type == SDL_MOUSEMOTION) {
        float xoffset = event.motion.xrel * camera->sensitivity;
        float yoffset = -event.motion.yrel *
                        camera->sensitivity; // y-coordinates are reversed

        std::cout << "X offset: " << xoffset << " y offset: " << yoffset
                  << std::endl;
        camera->yaw += xoffset;
        camera->pitch += yoffset;

        // Clamp pitch to prevent flipping
        if (camera->pitch > 89.0f)
            camera->pitch = 89.0f;
        if (camera->pitch < -89.0f)
            camera->pitch = -89.0f;

        glm::vec3 front;
        front.x =
            cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        front.y = sin(glm::radians(camera->pitch));
        front.z =
            sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        camera->front = glm::normalize(front);
    }
}

auto VulkanRendererSystem::update() -> void {
    auto engineStatePair = componentManager->getTComponentWithEntity<
        gim::ecs::components::EngineState::Component>(getEntities());
    auto triangleShaderPair = componentManager->getTComponentWithEntity<
        gim::ecs::components::Shader::ShaderBuilder>(getEntities());
    auto cameraPair =
        componentManager
            ->getTComponentWithEntity<gim::ecs::components::Camera::Component>(
                getEntities());

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

    shaderBuilder = triangleShaderComponent;
    camera = cameraComponent;

    if (!readyToFinishInitialization) {
        readyToFinishInitialization = true;

        finishCreatingGraphicsPipeline();
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            engineState->state = gim::ecs::components::EngineState::Quitting;
            return;
        } else if (event.type == SDL_WINDOWEVENT &&
                   event.window.event == SDL_WINDOWEVENT_RESIZED) {
        }

        processInput(event);
        handleMouseMotion(event);
    }

    drawFrame();
}

auto VulkanRendererSystem::insertEntity(Entity entity) -> void {
    entities.push_back(entity);
}

auto VulkanRendererSystem::removeEntity(Entity entity) -> void {
    entities.erase(std::remove(entities.begin(), entities.end(), entity),
                   entities.end());
}

auto VulkanRendererSystem::getEntities() -> std::vector<Entity> const & {
    return entities;
}
auto VulkanRendererSystem::setComponentManager(
    std::shared_ptr<ComponentManager> componentManager) -> void {
    componentManager = componentManager;
}
auto VulkanRendererSystem::getComponentManager()
    -> std::shared_ptr<ComponentManager> {
    return componentManager;
}

#pragma mark - Vulkan pipeline creation.
auto VulkanRendererSystem::finishCreatingGraphicsPipeline() -> void {
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createCommandBuffers();
    createSyncObjects();
}

[[nodiscard]] auto VulkanRendererSystem::findMemoryType(
    uint32_t typeFilter, VkMemoryPropertyFlags properties) const -> uint32_t {
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

auto VulkanRendererSystem::createVertexBuffer() -> void {
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(components::Shader::Vertex) *
                shaderBuilder->getVertices().size(),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(instance.device.device, &bufferInfo, nullptr,
                       &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

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

    if (vkAllocateMemory(instance.device.device, &allocInfo, nullptr,
                         &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(instance.device.device, vertexBuffer, vertexBufferMemory,
                       0);

    void *data;
    vkMapMemory(instance.device.device, vertexBufferMemory, 0, bufferInfo.size,
                0, &data);
    memcpy(data, shaderBuilder->getVertices().data(), (size_t)bufferInfo.size);
    vkUnmapMemory(instance.device.device, vertexBufferMemory);
}

auto VulkanRendererSystem::createGraphicsPipeline() -> void {
    auto builtShader = shaderBuilder->build();
    if (!shaderBuilder) {
        std::cout << "Required triangle shader not found!" << std::endl;
        return;
    }
    auto shader_stages = builtShader->getShaderStageCreateInfo(instance.device);

    auto shaderVertexBindings =
        shaderBuilder->getVertexBindingDescriptionsForDevice();
    auto shaderAttributes =
        shaderBuilder->getVertexAttributesDescriptionsForDevice();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount =
        static_cast<uint32_t>(shaderVertexBindings.size());
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(shaderAttributes.size());

    vertexInputInfo.pVertexBindingDescriptions = shaderVertexBindings.data();
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

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = instance.swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

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

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(instance.device, &pipeline_layout_info, nullptr,
                               &instance.data.pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount =
        static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates = dynamic_states.data();

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pVertexInputState = &vertexInputInfo;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.layout = instance.data.pipeline_layout;
    pipeline_info.renderPass = instance.data.render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(
            instance.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
            &instance.data.graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

auto VulkanRendererSystem::createFramebuffers() -> void {
    instance.data.swapchain_images = instance.swapchain.get_images().value();
    instance.data.swapchain_image_views =
        instance.swapchain.get_image_views().value();

    instance.data.framebuffers.resize(
        instance.data.swapchain_image_views.size());

    for (size_t i = 0; i < instance.data.swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {instance.data.swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = instance.data.render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = instance.swapchain.extent.width;
        framebuffer_info.height = instance.swapchain.extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(instance.device, &framebuffer_info, nullptr,
                                &instance.data.framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

auto VulkanRendererSystem::createCommandPool() -> void {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex =
        instance.device.get_queue_index(vkb::QueueType::graphics).value();

    if (vkCreateCommandPool(instance.device, &pool_info, nullptr,
                            &instance.data.command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

auto VulkanRendererSystem::createCommandBuffers() -> void {
    instance.data.command_buffers.resize(instance.data.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = instance.data.command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        (uint32_t)instance.data.command_buffers.size();

    if (vkAllocateCommandBuffers(instance.device, &allocInfo,
                                 instance.data.command_buffers.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < instance.data.command_buffers.size(); i++) {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(instance.data.command_buffers[i],
                                 &begin_info) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to begin recording command buffers!");
        }

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

        if (vkEndCommandBuffer(instance.data.command_buffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

auto VulkanRendererSystem::createSyncObjects() -> void {
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
        if (vkCreateSemaphore(instance.device, &semaphore_info, nullptr,
                              &instance.data.available_semaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(instance.device, &semaphore_info, nullptr,
                              &instance.data.finished_semaphore[i]) !=
                VK_SUCCESS ||
            vkCreateFence(instance.device, &fence_info, nullptr,
                          &instance.data.in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects!");
        }
    }
}

auto VulkanRendererSystem::recreate_swapchain() -> auto {
    vkDeviceWaitIdle(instance.device);
    vkDestroyCommandPool(instance.device, instance.data.command_pool, nullptr);

    for (auto framebuffer : instance.data.framebuffers) {
        vkDestroyFramebuffer(instance.device, framebuffer, nullptr);
    }

    instance.swapchain.destroy_image_views(instance.data.swapchain_image_views);

    instance.createSwapchain();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
}

auto VulkanRendererSystem::drawFrame() -> void {
    vkWaitForFences(
        instance.device, 1,
        &instance.data.in_flight_fences[instance.data.current_frame], VK_TRUE,
        UINT64_MAX);

    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        instance.device, instance.swapchain, UINT64_MAX,
        instance.data.available_semaphores[instance.data.current_frame],
        VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swapchain image!");
    }

    if (instance.data.image_in_flight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(instance.device, 1,
                        &instance.data.image_in_flight[image_index], VK_TRUE,
                        UINT64_MAX);
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
    submitInfo.pCommandBuffers = &instance.data.command_buffers[image_index];

    VkSemaphore signal_semaphores[] = {
        instance.data.finished_semaphore[instance.data.current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    vkResetFences(instance.device, 1,
                  &instance.data.in_flight_fences[instance.data.current_frame]);

    if (vkQueueSubmit(
            instance.data.graphics_queue, 1, &submitInfo,
            instance.data.in_flight_fences[instance.data.current_frame]) !=
        VK_SUCCESS) {
        std::cout << "failed to submit draw command buffer\n";
    }

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
} // namespace gim::ecs::systems
