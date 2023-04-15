#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/signature.hpp>
#include <iostream>

class TestComponent : public gim::ecs::IComponent {
  public:
	int x;

	TestComponent(int x) : x(x) {}
};

class TestSystem : public gim::ecs::ISystem {
  public:
	auto getSignature() -> std::shared_ptr<gim::ecs::Signature> override {
		auto signature = std::make_shared<gim::ecs::Signature>();
		signature->set<TestComponent>();
		return signature;
	}

	void update() override {
		for (auto &entity : getEntities()) {
			auto component = getComponent<TestComponent>(entity);
			component->x += 5;
		}
	}
};

TEST_CASE("ECS") {
	auto ecs = gim::ecs::ECS();

	auto e1 = ecs.createEntity();
	auto e2 = ecs.createEntity();

	ecs.registerSystem<TestSystem>();
	ecs.registerComponent<TestComponent>();

	auto c1 = std::make_shared<TestComponent>(5);
	auto c2 = std::make_shared<TestComponent>(10);

	ecs.addComponent(e1, c1);
	ecs.addComponent(e2, c2);

	CHECK(ecs.getComponent<TestComponent>(e1)->x == 5);
	CHECK(ecs.getComponent<TestComponent>(e2)->x == 10);

	ecs.update();

	CHECK(ecs.getComponent<TestComponent>(e1)->x == 10);
	CHECK(ecs.getComponent<TestComponent>(e2)->x == 15);
}
