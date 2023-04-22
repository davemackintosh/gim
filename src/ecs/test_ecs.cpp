#include "gim/ecs/engine/component_array.hpp"
#include "gim/ecs/engine/signature.hpp"
#include "gim/ecs/engine/testing.hpp"
#include <doctest/doctest.h>
#include <gim/ecs/ecs.hpp>
#include <iostream>

using namespace gim::ecs;

TEST_CASE("ECS") {
	auto ecs = ECS();

	auto e1 = ecs.createEntity();
	auto e2 = ecs.createEntity();

	ecs.registerSystem<TESTING::TestSystem>();
	ecs.registerComponent<TESTING::TestComponent>();

	auto c1 = std::make_shared<TESTING::TestComponent>(5);
	auto c2 = std::make_shared<TESTING::TestComponent>(10);

	ecs.addComponent(e1, c1);
	ecs.addComponent(e2, c2);

	CHECK(ecs.getComponent<TESTING::TestComponent>(e1)->x == 5);
	CHECK(ecs.getComponent<TESTING::TestComponent>(e2)->x == 10);

	ecs.update();

	CHECK(ecs.getComponent<TESTING::TestComponent>(e1)->x == 6);
	CHECK(ecs.getComponent<TESTING::TestComponent>(e2)->x == 11);
}
