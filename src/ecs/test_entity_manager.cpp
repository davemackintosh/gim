#include "gim/ecs/ecs.hpp"
#include "gim/ecs/signature.hpp"
#include <doctest/doctest.h>
#include <gim/ecs/component_array.hpp>
#include <gim/ecs/entity_manager.hpp>
#include <gim/ecs/testing.hpp>

TEST_CASE("entity-manager") {
	auto em = gim::ecs::EntityManager<10>();
	auto e1 = em.createEntity();

	CHECK(e1 == 0);

	// Create a signature for the entity.
	auto s1 = std::make_shared<gim::ecs::Signature>();
	s1->set<gim::ecs::TESTING::TestComponent>();
	em.setSignature(e1, s1);
	CHECK(em.getSignature(e1)->get<gim::ecs::TESTING::TestComponent>() == true);
	CHECK(em.getSignature(e1) != nullptr);

	// Create another entity, it should be 1.
	// Destroy the first entity, it should be 1.
	auto e2 = em.createEntity();
	CHECK(e2 == 1);

	em.destroyEntity(e1);
	CHECK(em.getSignature(e1) == nullptr);
}
