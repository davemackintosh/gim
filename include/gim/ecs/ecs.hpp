#pragma once

#include "gim/ecs/engine/component_manager.hpp"
#include "gim/ecs/engine/entity_manager.hpp"
#include "gim/ecs/engine/system_manager.hpp"
#include <gim/ecs/components/engine-state.hpp>
#include <memory>
#include <utility>

namespace gim::ecs {
class ECS {
  private:
    std::shared_ptr<EntityManager<100'00>> entityManager =
        std::make_shared<EntityManager<100'00>>();
    std::shared_ptr<ComponentManager> componentManager =
        std::make_shared<ComponentManager>();
    std::shared_ptr<SystemManager> systemManager =
        std::make_shared<SystemManager>(componentManager);

  public:
    auto createEntity() -> Entity { return entityManager->createEntity(); }

    auto destroyEntity(Entity entity) -> void {
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
        systemManager->entitySignatureChanged(entity, nullptr);
    }

    template <typename T> auto registerComponent() -> void {
        componentManager->registerComponent<T>();
    }

    template <typename T>
    auto addComponent(
        Entity entity,
        std::shared_ptr<gim::ecs::components::EngineState::Component> component)
        -> void {
        componentManager->addComponent(entity, std::move(component));
        auto signature = entityManager->getSignature(entity);
        signature->set<T>();
        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T> auto removeComponent(Entity entity) -> void {
        componentManager->removeComponent<T>(entity);
        auto signature = entityManager->getSignature(entity);
        signature->unset<T>();
        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    auto getComponent(Entity entity) -> std::shared_ptr<T> {
        return componentManager->getComponent<T>(entity);
    }

    template <typename T> auto registerSystem() -> void {
        systemManager->registerSystem<T>();
    }

    auto update() -> void { systemManager->update(); }
};
} // namespace gim::ecs
