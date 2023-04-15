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
  private:
	Signature signature;
	std::vector<Entity> entities;
	std::shared_ptr<ComponentManager> componentManager;

  public:
	Signature getSignature() override {
		signature.set<TestComponent>();
		return signature;
	}

	void update() override {
		for (auto const &entity : entities) {
			auto tc = componentManager->getComponent<TestComponent>(entity);
			tc->x++;
		}
	}
};

TEST_CASE("system-manager") {
	EntityManager em;
	SystemManager sm;
	ComponentManager cm;

	sm.registerSystem<TestSystem>();
	auto entity = em.createEntity();
	cm.registerComponent<TestComponent>();
	cm.addComponent(entity, std::make_shared<TestComponent>(1));

	auto system = sm.getSystem<TestSystem>();
	auto signature = system->getSignature();
	CHECK(signature.get<TestComponent>());
}
