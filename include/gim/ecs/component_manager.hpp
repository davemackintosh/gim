#pragma once

#include <assert.h>
#include <gim/ecs/component_array.hpp>
#include <gim/ecs/ecs.hpp>
#include <map>
#include <memory>
#include <string_view>

namespace gim::ecs {
class ComponentManager {
  private:
	std::map<std::string_view, std::shared_ptr<IComponentArray>>
		componentArrays;

#ifdef TEST
	// In order to test the private methods, we need to make them
	// public in the test environment.
  public:
#endif
	template <typename T> auto getComponentArray() -> ComponentArray<T> * {
		std::string_view typeName = typeid(T).name();

		assert(componentArrays.find(typeName) != componentArrays.end() &&
			   "Component not registered before use.");

		return std::static_pointer_cast<ComponentArray<T>>(
				   componentArrays.at(typeName))
			.get();
	}

  public:
	template <typename T> auto registerComponent() -> void {
		std::string_view typeName = typeid(T).name();

		assert(componentArrays.find(typeName) == componentArrays.end() &&
			   "Registering component type more than once.");

		componentArrays.insert(
			{typeName, std::make_shared<ComponentArray<T>>()});
	}

	template <typename T>
	auto addComponent(Entity entity, std::shared_ptr<T> component) -> void {
		getComponentArray<T>()->insertData(entity, component);
	}

	template <typename T> auto removeComponent(Entity entity) -> void {
		getComponentArray<T>()->removeData(entity);
	}

	template <typename T>
	auto getComponent(Entity entity) -> std::shared_ptr<T> {
		return getComponentArray<T>()->getData(entity);
	}
};
} // namespace gim::ecs
