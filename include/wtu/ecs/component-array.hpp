#pragma once

#include "./constants.hpp"
#include <array>
#include <optional>

// Component concept: POD types, moveable, default-constructible
template <typename T>
concept Component = std::is_trivially_copyable_v<T> &&
                    std::is_trivially_move_constructible_v<T> &&
                    std::is_default_constructible_v<T>;

template <Component T> class ComponentArray {
    std::array<std::optional<T>, MaxEntities> data;

  public:
    template <typename... Args>
    auto emplace(size_t entity_id, Args &&...args) -> T & {
        data[entity_id].emplace(std::forward<Args>(args)...);
        return *data[entity_id];
    }

    auto get(size_t entity_id) -> std::optional<T> & { return data[entity_id]; }

    void remove(size_t entity_id) { data[entity_id].reset(); }
};
