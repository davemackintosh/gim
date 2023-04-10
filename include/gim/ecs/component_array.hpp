#pragma once

#include <gim/ecs/ecs.hpp>
#include <memory>

namespace gim::ecs {

class IComponentArray {
  public:
	virtual ~IComponentArray() = default;
};

template <typename ComponentType> class ComponentArray {
  private:
	std::array<std::unique_ptr<ComponentType>, ECS_MAX_ENTITIES> components;
	std::array<Entity, ECS_MAX_ENTITIES> entities;
	std::array<int, ECS_MAX_ENTITIES> indices;
	int numComponents;

  public:
	ComponentArray() { numComponents = 0; };

	template <typename T>
	auto insertData(Entity entity, std::unique_ptr<T> component) -> void {
		// Check if the entity has a component.
		if (hasData(entity)) {
			return;
		}

		// Check if the component array is full.
		if (numComponents >= ECS_MAX_ENTITIES) {
			return;
		}

		// Insert the component.
		components[numComponents] = std::move(component);
		entities[numComponents] = entity;
		indices[entity] = numComponents;
		numComponents++;
	};

	auto removeData(Entity entity) -> void {
		// Check if the entity has a component.
		if (!hasData(entity)) {
			return;
		}

		// Remove the component.
		components[indices[entity]] = std::move(components[numComponents - 1]);
		entities[indices[entity]] = entities[numComponents - 1];
		indices[entities[numComponents - 1]] = indices[entity];
		numComponents--;
	};

	template <typename T> auto getData(Entity entity) -> std::unique_ptr<T> {
		// Check if the entity has a component.
		if (!hasData(entity)) {
			return nullptr;
		}

		// Return the component.
		return std::move(components[indices[entity]]);
	};

	auto hasData(Entity entity) -> bool {
		// Check if the entity has a component.
		if (indices[entity] >= numComponents) {
			return false;
		}

		// Check if the entity has a component.
		if (entities[indices[entity]] != entity) {
			return false;
		}

		// The entity has a component.
		return true;
	};
};
} // namespace gim::ecs
