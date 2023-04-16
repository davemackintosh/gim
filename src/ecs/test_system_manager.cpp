#include <doctest/doctest.h>
#include <gim/ecs/component_manager.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/entity_manager.hpp>
#include <gim/ecs/system_manager.hpp>

using namespace gim::ecs;

class TestComponent : public IComponent {
  public:
	int x;

	TestComponent(int x) : x(x) {}
};

class TestSystem : public ISystem {
  public:
	auto getSignature() -> std::shared_ptr<Signature> override {
		auto signature = std::make_shared<Signature>();
		signature->set<TestComponent>();
		return signature;
	}

	auto update() -> void override {
		for (auto const &entity : getEntities()) {
			auto tc = getComponent<TestComponent>(entity);
			tc->x++;
		}
	}
};

TEST_CASE("system-manager") {
	auto em = std::make_unique<EntityManager<10>>();
	auto cm = std::make_shared<ComponentManager>();
	auto sm = SystemManager(cm);

	sm.registerSystem<TestSystem>();
	auto entity = em->createEntity();
	cm->registerComponent<TestComponent>();
	cm->addComponent(entity, std::make_shared<TestComponent>(1));

	sm.update();

	CHECK(cm->getComponent<TestComponent>(entity)->x == 2);
}
