#include "gim/ecs/ecs.hpp"
#include <doctest/doctest.h>
#include <gim/ecs/entity_manager.hpp>

TEST_CASE("entity-manager") {
	gim::ecs::EntityManager em;
	auto e1 = em.createEntity();

	CHECK(e1 == 0);

	em.setSignature(e1, gim::ecs::Signature());
	CHECK(em.getSignature(e1) != nullptr);
	CHECK(*em.getSignature(e1) == gim::ecs::Signature());

	// Create another entity, it should be 1.
	// Destroy the first entity, it should be 1.
	auto e2 = em.createEntity();
	CHECK(e2 == 1);

	em.destroyEntity(e1);
	CHECK(em.getSignature(e1) == nullptr);

	// Create another entity, it should be 0
	// because we destroyed the first entity
	// which freed up the 0 index.
	auto e3 = em.createEntity();
	CHECK(e3 == 0);
	// e2 should be 1 still.
	CHECK(e2 == 1);
}
