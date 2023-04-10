#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/entity_manager.hpp>

using namespace gim::ecs;

TEST_CASE("entity-manager") {
	EntityManager em;
	auto e1 = em.createEntity();

	CHECK(e1 == 0);
	CHECK(em.getSignature(e1) == 0);

	em.setSignature(e1, 1);
	CHECK(em.getSignature(e1) == 1);

	// Create another entity, it should be 1.
	// Destroy the first entity, it should be 1.
	auto e2 = em.createEntity();
	CHECK(e2 == 1);

	em.destroyEntity(e1);
	CHECK(em.getSignature(e1) == 0);
	CHECK(em.getSignature(e2) == 0);

	// Create another entity, it should be 0.
	auto e3 = em.createEntity();
	CHECK(e3 == 0);

	// e2 should be 1 still.
	CHECK(e2 == 1);
}
