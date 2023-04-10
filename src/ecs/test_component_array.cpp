#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <memory>

using namespace gim::ecs;

TEST_CASE("component-array") {
	auto componentArray = ComponentArray<TESTING::TestComponent>();
	Entity entity = 0;
	auto component = std::make_unique<TESTING::TestComponent>(5);

	componentArray.insertData(entity, std::move(component));
	CHECK(componentArray.hasData(entity));
	CHECK(componentArray.getData<TESTING::TestComponent>(entity)->getValue() ==
		  5);

	componentArray.removeData(entity);
	CHECK(!componentArray.hasData(entity));
}
