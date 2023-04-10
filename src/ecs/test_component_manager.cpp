#include <doctest/doctest.h>
#include <gim/ecs/component_manager.hpp>

using namespace gim::ecs;

TEST_CASE("component-manager") {
	auto componentManager = ComponentManager();
	auto component = std::make_unique<TESTING::TestComponent>(5);
	componentManager.registerComponent<TESTING::TestComponent>();
	componentManager.addComponent(0, std::move(component));
	CHECK(componentManager.hasComponent<TESTING::TestComponent>(0));
	CHECK(
		componentManager.getComponent<TESTING::TestComponent>(0)->getValue() ==
		5);
	componentManager.removeComponent<TESTING::TestComponent>(0);
	CHECK(!componentManager.hasComponent<TESTING::TestComponent>(0));
}
