#include <gim/ecs/ecs.hpp>

namespace gim::ecs {
template <typename T> ComponentArray<T>::ComponentArray() { numComponents = 0; }

template <typename T>
auto ComponentArray<T>::insertData(Entity entity, T component) -> void {
	// Check if the entity already has a component.
	if (hasData(entity)) {
		return;
	}

	// Insert the component into the array.
	components[numComponents] = component;
	entities[numComponents] = entity;
	indices[entity] = numComponents;
	numComponents++;
}

template <typename T>
auto ComponentArray<T>::removeData(Entity entity) -> void {
	// Check if the entity has a component.
	if (!hasData(entity)) {
		return;
	}

	// Get the index of the component.
	int index = indices[entity];

	// Swap the last component with the component to be removed.
	components[index] = components[numComponents - 1];
	entities[index] = entities[numComponents - 1];
	indices[entities[index]] = index;

	// Remove the last component.
	numComponents--;
}

template <typename T> auto ComponentArray<T>::getData(Entity entity) -> T & {
	// Check if the entity has a component.
	if (!hasData(entity)) {
		return components[0];
	}

	// Return the component.
	return components[indices[entity]];
}

template <typename T> auto ComponentArray<T>::hasData(Entity entity) -> bool {
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
}
} // namespace gim::ecs
