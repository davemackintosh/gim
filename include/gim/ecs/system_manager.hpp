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

	auto getEntities() -> std::vector<Entity> { return entities; }
	auto insertEntity(Entity entity) -> void { entities.push_back(entity); }
	auto removeEntity(Entity entity) -> void {
		entities.erase(std::remove(entities.begin(), entities.end(), entity),
					   entities.end());
	}
};

class SystemManager {
  private:
	std::map<std::string_view, std::shared_ptr<ISystem>> systems;
	std::shared_ptr<ComponentManager> componentManager;

  public:
	SystemManager(std::shared_ptr<ComponentManager> componentManager) {
		this->componentManager = componentManager;
	}

	template <typename T> auto registerSystem() -> void {
		std::string_view typeName = typeid(T).name();

		assert(systems.find(typeName) == systems.end() &&
			   "Registering system type more than once.");

		auto system = std::make_shared<T>();
		system->componentManager = componentManager;
		systems.insert({typeName, system});
	}

	template <typename T> auto getSystem() -> T * {
		std::string_view typeName = typeid(T).name();

		assert(systems.find(typeName) != systems.end() &&
			   "System not registered before use.");

		return std::static_pointer_cast<T>(systems.at(typeName)).get();
	}

	auto entitySignatureChanged(Entity entity,
								std::shared_ptr<Signature> entitySignature)
		-> void {
		for (auto const &[key, system] : systems) {
			auto const &systemSignature = system->getSignature();
			if (entitySignature->subsetOf(systemSignature)) {
				system->insertEntity(entity);
			} else {
				system->removeEntity(entity);
			}
		}
	}

	auto update() -> void {
		for (auto const &[key, system] : systems) {
			system->update();
		}
	}
};
} // namespace gim::ecs
