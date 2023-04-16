#pragma once

#include <gim/ecs/component_manager.hpp>
#include <gim/ecs/entity_manager.hpp>
#include <gim/ecs/system_manager.hpp>
#include <memory>

namespace gim::ecs {
class ECS {
  private:
	std::shared_ptr<EntityManager<100'00>> entityManager;
	std::shared_ptr<ComponentManager> componentManager;
	std::shared_ptr<SystemManager> systemManager;

  public:
	ECS() {
		entityManager = std::make_unique<EntityManager<100'00>>();
		componentManager = std::make_unique<ComponentManager>();
		systemManager = std::make_shared<SystemManager>(componentManager);
	}

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
	auto addComponent(Entity entity, std::shared_ptr<T> component) -> void {
		componentManager->addComponent(entity, component);
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
