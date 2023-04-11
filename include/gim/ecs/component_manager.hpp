#pragma once

#include "gim/ecs/component_array.hpp"
#include <gim/ecs/ecs.hpp>
#include <map>
#include <memory>

namespace gim::ecs {
class ComponentManager {
  private:
	std::map<std::string, std::unique_ptr<IComponent>> components{};
	std::map<std::string, std::unique_ptr<IComponentArray>> componentArrays{};

  public:
	ComponentManager() = default;

	template <typename T> auto registerComponent() -> void {
		auto typeName = typeid(T).name();
		if (components.find(typeName) != components.end()) {
			return;
		}

		// components.insert({typeName, std::make_unique<T>()});
		componentArrays.insert(
			{typeName, std::make_unique<ComponentArray<T>>()});
	};

	template <typename T>
	auto addComponent(Entity entity, std::unique_ptr<T> component) -> void {
		auto typeName = typeid(T).name();
		if (components.find(typeName) == components.end()) {
			return;
		}

		auto componentArray =
			static_cast<ComponentArray<T> *>(componentArrays[typeName].get());
		componentArray->template insertData<T>(entity, std::move(component));
	};

	template <typename T> auto removeComponent(Entity entity) -> void {
		auto typeName = typeid(T).name();
		if (components.find(typeName) == components.end()) {
			return;
		}

		auto componentArray =
			static_cast<ComponentArray<T> *>(componentArrays[typeName].get());
		componentArray->removeData(entity);
	};

	template <typename T> auto hasComponent(Entity entity) -> bool {
		auto typeName = typeid(T).name();
		if (components.find(typeName) == components.end()) {
			return false;
		}

		auto componentArray =
			static_cast<ComponentArray<T> *>(componentArrays[typeName].get());
		return componentArray->template getData<T>(entity) != nullptr;
	};

	template <typename T>
	auto getComponent(Entity entity) -> std::unique_ptr<T> {
		auto typeName = typeid(T).name();
		if (components.find(typeName) == components.end()) {
			return nullptr;
		}

		auto componentArray =
			static_cast<ComponentArray<T> *>(componentArrays[typeName].get());
		return componentArray->template getData<T>(entity);
	};

	auto getComponentArray(std::string typeName)
		-> std::unique_ptr<IComponentArray> {
		if (components.find(typeName) == components.end()) {
			return nullptr;
		}

		return std::move(componentArrays[typeName]);
	};
};
} // namespace gim::ecs
