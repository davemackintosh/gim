#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <gim/ecs/component_manager.hpp>
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
	auto tc = std::make_shared<TestComponent>(1);
	cm.addComponent(e, tc);
	CHECK(cm.getComponent<TestComponent>(e)->x == 1);
}
