#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/engine/system_manager.hpp>

namespace gim::ecs::systems {
const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRendererSystem : public gim::ecs::ISystem {
  private:
    std::vector<Entity> entities;
    std::shared_ptr<ComponentManager> componentManager;

  public:
    VulkanRendererSystem() {
    }

    auto getSignature() -> std::shared_ptr<Signature> override {
        auto signature = std::make_shared<Signature>();
        signature->set<gim::ecs::components::EngineState::Component>();
        return signature;
    }

    auto update() -> void override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            } else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_RESIZED) {
            }
        }
        for (auto const &entity : entities) {
            auto es =
                getComponentManager()
                    ->getComponent<
                        gim::ecs::components::EngineState::Component>(entity);
        }

        int res = draw_frame(init, render_data);
        if (res != 0) {
            std::cout << "failed to draw frame \n";
            return -1;
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