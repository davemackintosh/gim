#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <memory>

using namespace gim::ecs;

class TestComponent : public IComponent {
  public:
	int value;

	TestComponent(int value) : value(value) {}
};

TEST_CASE("component-array") {
	auto componentArray = std::make_shared<ComponentArray<TestComponent>>();
	Entity entity = 0;
	auto component = std::make_shared<TestComponent>(TestComponent{5});

	componentArray->insertData(entity, component);
	CHECK(componentArray->getData(entity)->value == 5);

	componentArray->removeData(entity);
	CHECK(componentArray->getData(entity) == nullptr);
}
