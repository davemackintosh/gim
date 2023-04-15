#pragma once

#include <assert.h>
#include <gim/ecs/ecs.hpp>
#include <map>
#include <memory>

namespace gim::ecs {
class SystemManager {
  private:
	std::map<std::string_view, std::shared_ptr<ISystem>> systems;

  public:
	template <typename T> auto registerSystem() -> void {
		std::string_view typeName = typeid(T).name();

		assert(systems.find(typeName) == systems.end() &&
			   "Registering system type more than once.");

		systems.insert({typeName, std::make_shared<T>()});
	}

	template <typename T> auto getSystem() -> T * {
		std::string_view typeName = typeid(T).name();

		assert(systems.find(typeName) != systems.end() &&
			   "System not registered before use.");

		return std::static_pointer_cast<T>(systems.at(typeName)).get();
	}

	auto entitySignatureChanged(Entity entity, Signature entitySignature)
		-> void {
		for (auto const &[key, system] : systems) {
			auto const &systemSignature = system->getSignature();
			if ((entitySignature & systemSignature) == systemSignature) {
				system->addEntity(entity);
			} else {
				system->removeEntity(entity);
			}
		}
	}

	auto update() -> void {
		for (auto const &[key, system] : systems) {
			system->update();
		}
	}
};
} // namespace gim::ecs
