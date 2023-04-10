#pragma once

#include <array>
#include <bitset>
#include <cstdint>

namespace gim::ecs {

const int ECS_MAX_ENTITIES = 5000;
const int ECS_MAX_COMPONENTS = 32;

typedef uint32_t Entity;
typedef std::bitset<ECS_MAX_COMPONENTS> Signature;

// A Component is just a placeholder class which is used to identify
// a component type in the ComponentArray and ComponentManager.
class Component {
  public:
	virtual ~Component() = default;

	// No copy or move.
	Component(const Component &) = delete;
	Component(Component &&) = delete;
	auto operator=(const Component &) -> Component & = delete;
	auto operator=(Component &&) -> Component & = delete;

  protected:
	Component() = default;
};

template <typename T> class ComponentArray {
  private:
	std::array<T, ECS_MAX_ENTITIES> components;
	std::array<Entity, ECS_MAX_ENTITIES> entities;
	std::array<int, ECS_MAX_ENTITIES> indices;
	int numComponents;

  public:
	ComponentArray();
	auto insertData(Entity entity, T component) -> void;
	auto removeData(Entity entity) -> void;
	auto getData(Entity entity) -> T &;
	auto hasData(Entity entity) -> bool;
};
} // namespace gim::ecs
