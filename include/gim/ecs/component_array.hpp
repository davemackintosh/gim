#pragma once

#include <algorithm>
#include <gim/ecs/entity_manager.hpp>
#include <memory>
#include <vector>

namespace gim::ecs {

// A Component is just a placeholder class which is used to identify
// a component type in the ComponentArray and ComponentManager.
class IComponent {
  public:
	virtual ~IComponent() = default;
};

class IComponentArray {
  public:
	virtual ~IComponentArray() = default;
	// There is no need to virtualise the methods below, because
	// they are not called from the base class pointer. This is
	// just a way to satisfy the compiler.
};

/**
 * @brief A component array is a contiguous memory array of components of a
 * particular type.
 */
template <typename ComponentType>
class ComponentArray : public IComponentArray {
  private:
	// Store the components in contiguous memory and make sure
	// that the ownership of the component remains at the component source.
	std::vector<std::shared_ptr<ComponentType>> components;
	// Store the entities in contiguous memory.
	std::vector<Entity> entities;

  public:
	ComponentArray() = default;
	~ComponentArray() = default;

	void insertData(Entity entity, std::shared_ptr<ComponentType> component) {
		// Add the entity to the entity vector.
		entities.push_back(entity);
		// Add the component to the component vector.
		components.push_back(component);
	}

	std::shared_ptr<ComponentType> getData(Entity entity) {
		// Find the index of the entity.
		auto it = std::find(entities.begin(), entities.end(), entity);
		// Get the index of the entity.
		auto index = std::distance(entities.begin(), it);

		// Check if the index is valid.
		if (index >= entities.size()) {
			return nullptr;
		}

		// Return the component at the index.
		return components[index];
	}

	void removeData(Entity entity) {
		auto it = std::find(entities.begin(), entities.end(), entity);
		auto index = std::distance(entities.begin(), it);
		entities.erase(entities.begin() + index);
		components.erase(components.begin() + index);
	}
};
} // namespace gim::ecs
