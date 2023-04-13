#include "gim/ecs/component_manager.hpp"
#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <memory>

using namespace gim::ecs;

class TestComponent : public IComponent {
  public:
	int x;

	TestComponent(int x) : x(x) {}
};

TEST_CASE("component-manager") {
	ComponentManager cm;

	cm.registerComponent<TestComponent>();
	CHECK(cm.getComponentArray<TestComponent>() != nullptr);

	Entity e = 0;
	TestComponent tc{1};
	cm.addComponent(e, tc);
	CHECK(cm.getComponent<TestComponent>(e).x == 1);
}
