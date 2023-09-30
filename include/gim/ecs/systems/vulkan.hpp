#pragma once

#include "gim/ecs/engine/entity_manager.hpp"
#include "vulkan/vulkan_core.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL_surface.h>
#include <VkBootstrap.h>
#include <algorithm>
#include <gim/ecs/components/engine-state.hpp>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/engine/system_manager.hpp>
#include <gim/engine.hpp>
#include <gim/vulkan/init.hpp>
#include <gim/vulkan/utils.hpp>
#include <memory>
#include <utility>

namespace gim::ecs::systems {
const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRendererSystem : public gim::ecs::ISystem {
  private:
    // ECS.
    std::vector<Entity> entities;
    std::shared_ptr<ComponentManager> componentManager;

    // Vulkan.
    gim::vulkan::Instance instance;

  public:
    VulkanRendererSystem() : instance(gim::vulkan::Instance()) {}

#pragma mark - ECS

    auto getSignature() -> std::shared_ptr<Signature> override {
        auto signature = std::make_shared<Signature>();
        signature->set<gim::ecs::components::EngineState::Component>();

        return signature;
    }

    auto update() -> void override {
        auto engineStatePair = componentManager->getTComponentWithEntity<
            gim::ecs::components::EngineState::Component>(getEntities());

        if (!engineStatePair.has_value()) {
            return;
        }

        auto [_, engineState] = engineStatePair.value();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                engineState->state =
                    gim::ecs::components::EngineState::Quitting;
                return;
            } else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_RESIZED) {
            }
        }
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
};
} // namespace gim::ecs::systems