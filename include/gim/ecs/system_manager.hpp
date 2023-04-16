#pragma once

#include <assert.h>
#include <gim/ecs/component_manager.hpp>
#include <map>
#include <memory>
#include <string_view>
#include <typeinfo>
#include <vector>

namespace gim::ecs {
class ISystem {
  private:
	std::vector<Entity> entities;

  public:
	std::shared_ptr<ComponentManager> componentManager;
	virtual ~ISystem() = default;
	virtual auto getSignature() -> std::shared_ptr<Signature> = 0;
	virtual void update() = 0;

	// Functions to use in the update methods of the derived systems.
	auto getEntities() -> std::vector<Entity> { return entities; }
	template <typename T>
	auto getComponent(Entity entity) -> std::shared_ptr<T> {
		return componentManager->getComponent<T>(entity);
	}
	auto insertEntity(Entity entity) -> void { entities.push_back(entity); }
	auto removeEntity(Entity entity) -> void {
		entities.erase(std::remove(entities.begin(), entities.end(), entity),
					   entities.end());
	}
};

template <class T>
concept System = std::is_base_of_v<ISystem, T>;

class SystemManager {
  private:
	std::vector<std::unique_ptr<ISystem>> systems;
	std::shared_ptr<ComponentManager> componentManager;
	template <System T> auto findSystem() -> int {
		return std::find_if(systems.begin(), systems.end(),
							[&](const std::unique_ptr<ISystem> &system) {
								return typeid(system.get()) == typeid(T);
							}) -
			   systems.begin();
	}
	template <System T> auto systemRegistered() -> bool {
		return findSystem<T>() != systems.size();
	}

  public:
	SystemManager(std::shared_ptr<ComponentManager> componentManager) {
		this->componentManager = componentManager;
	}

	template <System T> auto registerSystem() -> void {
		assert(!systemRegistered<T>() && "Registering system more than once.");

		systems.emplace_back(std::make_unique<T>());
		systems.back()->componentManager = componentManager;
	}

	auto entitySignatureChanged(Entity entity,
								std::shared_ptr<Signature> entitySignature)
		-> void {
		for (auto const &system : systems) {
			auto const &systemSignature = system->getSignature();

			if (entitySignature->subsetOf(systemSignature)) {
				system->insertEntity(entity);
			} else {
				system->removeEntity(entity);
			}
		}
	}

	auto update() -> void {
		for (auto const &system : systems) {
			system->update();
		}
	}
};
} // namespace gim::ecs
