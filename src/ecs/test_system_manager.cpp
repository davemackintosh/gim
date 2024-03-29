#include "gim/ecs/engine/component_manager.hpp"
#include "gim/ecs/engine/entity_manager.hpp"
#include "gim/ecs/engine/system_manager.hpp"
#include "gim/ecs/engine/testing.hpp"
#include <doctest/doctest.h>
#include <gim/ecs/ecs.hpp>
#include <memory>

using namespace gim::ecs;

TEST_CASE("system-manager") {
	auto em = std::make_unique<EntityManager<10>>();
	auto cm = std::make_shared<ComponentManager>();
	auto sm = SystemManager(cm);

	cm->registerComponent<TESTING::TestComponent>();
	sm.registerSystem<TESTING::TestSystem>();
	auto entity = em->createEntity();
	cm->addComponent(entity, std::make_shared<TESTING::TestComponent>(1));
	sm.entitySignatureChanged(entity, em->getSignature(entity));

	sm.update();

	CHECK(cm->getComponent<TESTING::TestComponent>(entity)->x == 2);
}
