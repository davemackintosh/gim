#pragma once

#include <assert.h>
#include <gim/ecs/component_manager.hpp>
#include <map>
#include <memory>
#include <string_view>
#include <typeinfo>
#include <vector>

namespace gim::ecs {
/**
 * @brief The ISystem class is the base class for all systems.
 */
class ISystem {
  public:
	virtual ~ISystem() = default;
	virtual auto getSignature() -> std::shared_ptr<Signature> = 0;
	virtual auto self() -> ISystem *const = 0;
	virtual void update() = 0;
	virtual auto getEntities() -> std::vector<Entity> const & = 0;
	virtual void insertEntity(Entity entity) = 0;
	virtual void removeEntity(Entity entity) = 0;
	virtual void setComponentManager(std::shared_ptr<ComponentManager> cm) = 0;
	virtual auto getComponentManager() -> std::shared_ptr<ComponentManager> = 0;
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
		this->componentManager = componentManager;
	}

	template <System T> auto registerSystem() -> void {
		assert(!systemRegistered<T>() && "Registering system more than once.");

		systems.emplace_back(std::make_unique<T>());
		systems.back()->setComponentManager(componentManager);
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
			system->self()->update();
		}
	}
};
} // namespace gim::ecs
