#pragma once

#include "gim/ecs/component_array.hpp"
#include <assert.h>
#include <gim/ecs/ecs.hpp>
#include <map>
#include <memory>

namespace gim::ecs {
class ComponentManager {
  private:
	std::map<const char *, std::unique_ptr<IComponentArray>> componentArrays;

#ifdef TEST
	// In order to test the private methods, we need to make them
	// public in the test environment.
  public:
#endif
	template <typename T> IComponentArray *getComponentArray() {
		const char *typeName = typeid(T).name();

		assert(componentArrays.find(typeName) != componentArrays.end() &&
			   "Component not registered before use.");

		return componentArrays[typeName].get();
	}

  public:
	template <typename T> auto registerComponent() -> void {
		const char *typeName = typeid(T).name();

		assert(componentArrays.find(typeName) == componentArrays.end() &&
			   "Registering component type more than once.");

		componentArrays.insert(
			{typeName, std::make_unique<ComponentArray<T>>()});
	}

	template <typename T>
	auto addComponent(Entity entity, T component) -> void {
		getComponentArray<T>()->insertData(entity, component);
	}

	template <typename T> auto removeComponent(Entity entity) -> void {
		getComponentArray<T>()->removeData(entity);
	}

	template <typename T> auto getComponent(Entity entity) -> T & {
		return getComponentArray<T>()->getData(entity);
	}
};
} // namespace gim::ecs
