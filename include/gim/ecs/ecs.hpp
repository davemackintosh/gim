#pragma once

#include <array>
#include <bitset>
#include <cstdint>

namespace gim::ecs {

const int ECS_MAX_ENTITIES = 5000;
const int ECS_MAX_COMPONENTS = 32;

typedef uint32_t Entity;
typedef std::bitset<ECS_MAX_COMPONENTS> Signature;

class EntityManager {
  private:
	std::array<Entity, ECS_MAX_ENTITIES> entities;
	std::array<Signature, ECS_MAX_ENTITIES> signatures;
	int numEntities;

  public:
	EntityManager();
	auto createEntity() -> Entity;
	auto destroyEntity(Entity entity) -> void;
	auto setSignature(Entity entity, Signature signature) -> void;
	auto getSignature(Entity entity) -> Signature;
};
} // namespace gim::ecs
