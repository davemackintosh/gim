#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <gim/ecs/component_manager.hpp>
#include <gim/ecs/testing.hpp>
#include <memory>

using namespace gim::ecs;

TEST_CASE("component-manager") {
	ComponentManager cm;

	cm.registerComponent<TESTING::TestComponent>();
	CHECK(cm.getComponentArray<TESTING::TestComponent>() != nullptr);

	Entity e = 0;
	auto tc = std::make_shared<TESTING::TestComponent>(1);
	cm.addComponent(e, tc);
	CHECK(cm.getComponent<TESTING::TestComponent>(e)->x == 1);
}
