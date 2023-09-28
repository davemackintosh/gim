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
#include <iostream>
#include <memory>
#include <utility>

namespace gim::ecs::systems {
const int MAX_FRAMES_IN_FLIGHT = 2;
const int WIDTH = 800;
const int HEIGHT = 600;

#define SDLMustBeTrue(expr)                                                    \
    if (!(expr)) {                                                             \
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;             \
        exit(EXIT_FAILURE);                                                    \
    }

class VulkanRendererSystem : public gim::ecs::ISystem {
  private:
    // ECS.
    std::vector<Entity> entities;
    std::shared_ptr<ComponentManager> componentManager;

    // Windowing.
    SDL_Window *window;
    VkSurfaceKHR *surface;
    VkInstance *instance;

  public:
    VulkanRendererSystem() {
        createVulkanInstance();
        createWindow();
    }

    auto getSignature() -> std::shared_ptr<Signature> override {
        auto signature = std::make_shared<Signature>();
        signature->set<gim::ecs::components::EngineState::Component>();
        return signature;
    }

    auto update() -> void override {
        auto engineStatePair = getComponentWithEntity<
            gim::ecs::components::EngineState::Component>();

        if (engineStatePair.first == nullptr) {
            return;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                engineStatePair.second->state =
                    gim::ecs::components::EngineState::Quitting;
                return;
            } else if (event.type == SDL_WINDOWEVENT &&
                       event.window.event == SDL_WINDOWEVENT_RESIZED) {
            }
        }
    }

#pragma mark - ECS

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

    // Try to find a component in the list of entities
    // and return the entity and component as a tuple.
    template <typename Component>
    auto getComponentWithEntity()
        -> std::pair<gim::ecs::Entity *, std::shared_ptr<Component>> {
        auto entities = getEntities();
        auto componentManager = getComponentManager();
        std::shared_ptr<Component> component;

        // Try to find a component in the list of entities.
        auto entity = std::find_if(
            entities.begin(), entities.end(),
            [&componentManager,
             &component](gim::ecs::Entity const &entity) -> bool {
                component = componentManager->getComponent<Component>(entity);

                return component != nullptr;
            });

        if (entity == entities.end()) {
            return std::make_pair(nullptr, nullptr);
        }

        return std::make_pair(&*entity, component);
    }

#pragma mark - Vulkan

    auto getWindow() -> SDL_Window * { return window; }
    auto getSurface() -> VkSurfaceKHR * { return surface; }
    auto getInstance() -> VkInstance * { return instance; }
    auto createVulkanInstance() -> void {
        vkb::InstanceBuilder instanceBuilder;
        auto builderResult = instanceBuilder.set_app_name(ENGINE_NAME)
                                 .request_validation_layers(true)
                                 .use_default_debug_messenger()
                                 .require_api_version(1, 2, 0)
                                 .build();

        vkb::Instance vkbInstance{builderResult.value()};
        instance = &vkbInstance.instance;
    }
    auto createSurface(SDL_Window *window) -> void {
        SDLMustBeTrue(SDL_Vulkan_CreateSurface(window, *instance, surface));
    }

#pragma mark - SDL

    auto createWindow() -> void {
        SDLMustBeTrue(SDL_Init(SDL_INIT_EVERYTHING) == 0);

        window = SDL_CreateWindow("Mountain", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                  SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                      SDL_WINDOW_SHOWN);
        SDLMustBeTrue(window != nullptr);

        VkSurfaceKHR rawSurface;
        SDLMustBeTrue(SDL_Vulkan_CreateSurface(window, *instance, &rawSurface));
        surface = &rawSurface;
    }
};
} // namespace gim::ecs::systems