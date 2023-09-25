#pragma once

#include <cassert>
#include <gim/ecs/engine/component_manager.hpp>
#include <gim/ecs/engine/entity_manager.hpp>
#include <gim/ecs/engine/signature.hpp>
#include <map>
#include <memory>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <vector>

namespace gim::ecs {
/**
 * @brief The ISystem class is the base class for all systems.
 */
class SystemInterface {
  public:
	virtual ~SystemInterface() = default;
	virtual auto getSignature() -> std::shared_ptr<Signature>;
	virtual auto self() -> SystemInterface *const = 0;
	virtual void update() = 0;
	virtual auto getEntities() -> std::vector<Entity> const & = 0;
	virtual void insertEntity(Entity entity) = 0;
	virtual void removeEntity(Entity entity) = 0;
	virtual void setComponentManager(std::shared_ptr<ComponentManager> cm) = 0;
	virtual auto getComponentManager() -> std::shared_ptr<ComponentManager> = 0;
};

class ISystem : public SystemInterface {
  private:
	std::vector<Entity> entities;
	std::shared_ptr<ComponentManager> componentManager;

  public:
	auto self() -> ISystem *const override { return this; }

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
	explicit SystemManager(std::shared_ptr<ComponentManager> componentManager) {
		this->componentManager = std::move(componentManager);
	}

	template <System T> auto registerSystem() -> void {
		assert(!systemRegistered<T>() && "Registering system more than once.");

		systems.emplace_back(std::make_unique<T>());
		systems.back()->setComponentManager(componentManager);
	}

	auto
	entitySignatureChanged(Entity entity,
						   const std::shared_ptr<Signature> &entitySignature)
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
			system->self()->update();
		}
	}
};
} // namespace gim::ecs
