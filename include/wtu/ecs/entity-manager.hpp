#pragma once

#include "./constants.hpp"
#include <optional>
#include <vector>

class EntityManager {
    std::vector<bool> alive_entities;

  public:
    EntityManager();

    auto create_entity() -> std::optional<size_t>;
    void destroy_entity(size_t entityId);
    [[nodiscard]] auto is_alive(size_t entityId) const -> bool;
};
