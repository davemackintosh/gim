#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <gim/ecs/ecs.hpp>

using namespace gim::ecs;

TEST_CASE("component-array") {
	auto componentArray = ComponentArray<int>();
	Entity entity = 0;
	int component = 5;

	componentArray.insertData(entity, component);
	CHECK(componentArray.getData(entity) == 5);
	CHECK(componentArray.hasData(entity) == true);

	componentArray.removeData(entity);
	CHECK(componentArray.hasData(entity) == false);
}
