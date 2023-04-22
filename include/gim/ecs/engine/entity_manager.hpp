#pragma once

#include <array>
#include <gim/ecs/engine/signature.hpp>
#include <memory>

namespace gim::ecs {
typedef uint32_t Entity;
/**
 * @brief The entity manager class.
 *
 * @tparam ECS_MAX_ENTITIES The maximum number of entities that can exist at
 * any given time.
 */
template <int ECS_MAX_ENTITIES> class EntityManager {
  private:
	std::array<Entity, ECS_MAX_ENTITIES> entities;
	std::array<std::shared_ptr<Signature>, ECS_MAX_ENTITIES> signatures;
	int numEntities;

  public:
	/**
	 * @brief Construct a new Entity Manager object and initialize the
	 * entities array with all possible entities so that we can keep the
	 * memory contiguous and allow for fast iteration and data locality.
	 */
	EntityManager() : entities(), signatures(), numEntities(0) {
		for (int i = 0; i < ECS_MAX_ENTITIES; i++) {
			entities[i] = i;
			signatures[i] = std::make_shared<Signature>();
		}
	};

	/**
	 * @brief Create an entity and return it.
	 *
	 * @return Entity The created entity.
	 */
	auto createEntity() -> Entity {
		// Check if there are any available entities.
		if (numEntities == ECS_MAX_ENTITIES) {
			return ECS_MAX_ENTITIES;
		}

		// Return the next available entity.
		return entities[numEntities++];
	};

	/**
	 * @brief Destroy an entity.
	 *
	 * @param entity The entity to destroy.
	 */
	auto destroyEntity(Entity entity) -> void {
		// Move the destroyed entity to the end of the array and decrease the
		// numEntities, we also need to reset the signature of the entity.
		signatures.at(entity).reset();
		std::swap(entities.at(entity), entities.at(numEntities - 1));
		numEntities--;
	};

	/**
	 * @brief Set the signature of an entity.
	 *
	 * @param entity The entity to set the signature of.
	 * @param signature The signature to set.
	 */
	auto setSignature(Entity entity,
					  const std::shared_ptr<Signature> &signature) -> void {
		// Set the signature of the entity.
		signatures.at(entity) = signature;
	};

	/**
	 * @brief Get the signature of an entity.
	 *
	 * @param entity The entity to get the signature of.
	 * @return Signature The signature of the entity.
	 */
	auto getSignature(Entity entity) -> std::shared_ptr<Signature> {
		// Return the signature of the entity.
		return signatures.at(entity);
	}
};
} // namespace gim::ecs
