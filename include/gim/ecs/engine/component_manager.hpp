#pragma once

#include <cassert>
#include <gim/ecs/engine/component_array.hpp>
#include <gim/ecs/engine/entity_manager.hpp>
#include <map>
#include <memory>
#include <ranges>
#include <string_view>
#include <typeinfo>

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
    ComponentManager() { componentArrays.clear(); }
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

    auto entityDestroyed(Entity entity) -> void {
        for (auto const &[key, componentArray] : componentArrays) {
            componentArray->entityDestroyed(entity);
        }
    }

    // Try to find a component in the list of entities
    // and return the entity and component as a tuple.
    template <typename Component>
    auto getTComponentWithEntity(const std::vector<gim::ecs::Entity> entities)
        -> std::optional<
            std::pair<gim::ecs::Entity, std::shared_ptr<Component>>> {

        // Try to find a component in the list of entities.
        auto entity =
            std::find_if(entities.begin(), entities.end(),
                         [this](gim::ecs::Entity const &entity) -> bool {
                             return getComponent<Component>(entity) != nullptr;
                         });

        if (entity == entities.end()) {
            return std::nullopt;
        }

        auto result = std::make_pair(*entity, getComponent<Component>(*entity));

        return std::make_optional(result);
    }

    template <typename Component>
    auto getAllTComponentsAndEntities(
        std::vector<std::shared_ptr<gim::ecs::Entity>> entities)
        -> std::vector<
            std::pair<gim::ecs::Entity *, std::shared_ptr<Component>>> {
        return entities |
               std::views::filter(
                   [this](gim::ecs::Entity const &entity) -> bool {
                       return getComponent<Component>(entity) != nullptr;
                   }) |
               std::views::transform(
                   [this](gim::ecs::Entity const &entity)
                       -> std::pair<gim::ecs::Entity *,
                                    std::shared_ptr<Component>> {
                       return std::make_pair(&entity,
                                             getComponent<Component>(entity));
                   });
    }
};
} // namespace gim::ecs
