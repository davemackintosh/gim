#include <gim/ecs/ecs.hpp>

namespace gim::ecs {
EntityManager::EntityManager() : numEntities(0) {
	// Pack the entities array with all possible entities.
	// This is to keep the memory contiguous and allow for
	// fast iteration and data locality.
	for (int i = 0; i < ECS_MAX_ENTITIES; i++) {
		entities[i] = i;
	}
}

auto EntityManager::createEntity() -> Entity { return entities[numEntities++]; }
auto EntityManager::destroyEntity(Entity entity) -> void {
	// Move the destroyed entity to the end of the array and decrease the
	// numEntities, we also need to reset the signature of the entity.
	signatures[entity].reset();
	std::swap(entities[entity], entities[numEntities - 1]);
	numEntities--;
}
auto EntityManager::setSignature(Entity entity, Signature signature) -> void {
	signatures[entity] = signature;
}
auto EntityManager::getSignature(Entity entity) -> Signature {
	return signatures[entity];
}
} // namespace gim::ecs
