#pragma once

#include "component_manager.hpp"
#include "entity_manager.hpp"
#include "system_manager.hpp"

namespace gim::ecs::TESTING {
class TestComponent : public IComponent {
  public:
	int x;

	explicit TestComponent(int x) : x(x) {}
};

class TestSystem : public ISystem {
  private:
	std::vector<Entity> entities;
	std::shared_ptr<ComponentManager> componentManager;

  public:
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
};
} // namespace gim::ecs::TESTING
