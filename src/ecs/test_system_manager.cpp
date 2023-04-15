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
	std::vector<Entity> entities;

  public:
	auto getSignature() -> std::shared_ptr<Signature> override {
		auto signature = std::make_shared<Signature>();
		signature->set<TestComponent>();
		return signature;
	}

	auto update() -> void override {
		for (auto const &entity : entities) {
			auto tc = componentManager->getComponent<TestComponent>(entity);
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
};

TEST_CASE("system-manager") {
	auto em = std::make_unique<EntityManager<10>>();
	auto cm = std::make_shared<ComponentManager>();
	auto sm = SystemManager(cm);

	sm.registerSystem<TestSystem>();
	auto entity = em->createEntity();
	cm->registerComponent<TestComponent>();
	cm->addComponent(entity, std::make_shared<TestComponent>(1));

	auto system = sm.getSystem<TestSystem>();
	auto signature = system->getSignature();
	CHECK(signature->get<TestComponent>());
}
