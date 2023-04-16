#pragma once

#include <gim/ecs/component_manager.hpp>
#include <gim/ecs/entity_manager.hpp>
#include <gim/ecs/system_manager.hpp>

namespace gim::ecs::TESTING {
class TestComponent : public IComponent {
  public:
	int x;

	TestComponent(int x) : x(x) {}
};

class TestSystem : public ISystem {
  private:
	std::vector<Entity> entities;
	std::shared_ptr<ComponentManager> componentManager;

  public:
	auto self() -> TestSystem *const override { return this; }
	auto getSignature() -> std::shared_ptr<Signature> override {
		auto signature = std::make_shared<Signature>();
		signature->set<TestComponent>();
		return signature;
	}

	auto update() -> void override {
		for (auto const &entity : entities) {
			auto tc =
				getComponentManager()->getComponent<TestComponent>(entity);
			tc->x++;
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
} // namespace gim::ecs::TESTING
