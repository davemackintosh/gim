#pragma once

#include <gim/ecs/engine/system_manager.hpp>
#include <VkBootstrap.h>
#include <gim/ecs/components/vertex.hpp>

namespace gim::ecs::systems {
class VulkanRendererSystem : public gim::ecs::ISystem {
  private:
    std::vector<Entity> entities;
    std::shared_ptr<ComponentManager> componentManager;
  public:
    VulkanRendererSystem(){

    };
    ~VulkanRendererSystem() = default;

    auto getSignature() -> std::shared_ptr<Signature> override {
        auto signature = std::make_shared<Signature>();
        signature->set<gim::ecs::components::Vertex>();
        return signature;
    }

    auto update() -> void override {
        for (auto const &entity : entities) {
            auto tc =
                getComponentManager()->getComponent<gim::ecs::components::Vertex>(entity);
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
}