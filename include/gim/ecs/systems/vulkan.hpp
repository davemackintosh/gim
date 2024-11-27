#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL_surface.h>
#include <VkBootstrap.h>
#include <gim/ecs/components/camera.hpp>
#include <gim/ecs/components/engine-state.hpp>
#include <gim/ecs/components/shader-base.hpp>
#include <gim/ecs/components/triangle-shader.hpp>
#include <gim/ecs/engine/entity_manager.hpp>
#include <gim/ecs/engine/system_manager.hpp>
#include <gim/engine.hpp>
#include <gim/vulkan/instance.hpp>
#include <gim/vulkan/utils.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace gim::ecs::systems {
const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRendererSystem : public gim::ecs::ISystem {
  private:
    // ECS.
    std::vector<Entity> entities;
    std::shared_ptr<ComponentManager> componentManager;

    // Vulkan.
    gim::vulkan::Instance instance;
    VkBuffer vertexBuffer;
    bool readyToFinishInitialization = false;
    VkDeviceMemory vertexBufferMemory;

    // Engine.
    std::shared_ptr<gim::ecs::components::Shader::ShaderBuilder> shaderBuilder;
    std::shared_ptr<gim::ecs::components::Camera::Component> camera;

  public:
    VulkanRendererSystem();
    VulkanRendererSystem(const VulkanRendererSystem &) = default;
    VulkanRendererSystem(VulkanRendererSystem &&) = delete;
    auto operator=(const VulkanRendererSystem &)
        -> VulkanRendererSystem & = default;
    auto operator=(VulkanRendererSystem &&) -> VulkanRendererSystem & = delete;
    ~VulkanRendererSystem() override;

#pragma mark - ECS

    auto getSignature() -> std::shared_ptr<Signature> override;
    auto processInput(SDL_Event &event) -> void;
    auto handleMouseMotion(SDL_Event &event) -> void;
    auto update() -> void override;
    auto insertEntity(Entity entity) -> void override;
    auto removeEntity(Entity entity) -> void override;
    auto getEntities() -> std::vector<Entity> const & override;
    auto setComponentManager(std::shared_ptr<ComponentManager> componentManager)
        -> void override;
    auto getComponentManager() -> std::shared_ptr<ComponentManager> override;

#pragma mark - Vulkan pipeline creation.
    auto finishCreatingGraphicsPipeline() -> void;

    [[nodiscard]] auto findMemoryType(uint32_t typeFilter,
                                      VkMemoryPropertyFlags properties) const
        -> uint32_t;
    auto createVertexBuffer() -> void;
    auto createGraphicsPipeline() -> void;
    auto createFramebuffers() -> void;
    auto createCommandPool() -> void;
    auto createCommandBuffers() -> void;
    auto createSyncObjects() -> void;
    auto recreate_swapchain() -> auto;
    auto drawFrame() -> void;
};
} // namespace gim::ecs::systems
