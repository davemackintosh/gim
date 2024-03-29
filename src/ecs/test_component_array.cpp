#include "gim/ecs/engine/component_array.hpp"
#include "gim/ecs/engine/testing.hpp"
#include <doctest/doctest.h>
#include <memory>

using namespace gim::ecs;

TEST_CASE("component-array") {
	auto componentArray =
		std::make_shared<ComponentArray<TESTING::TestComponent>>();
	Entity entity = 0;
	auto component =
		std::make_shared<TESTING::TestComponent>(TESTING::TestComponent{5});

	componentArray->insertData(entity, component);
	CHECK(componentArray->getData(entity)->x == 5);

	componentArray->removeData(entity);
	CHECK(componentArray->getData(entity) == nullptr);
}
