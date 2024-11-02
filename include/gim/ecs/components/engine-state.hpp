#pragma once

#include <gim/ecs/engine/component_array.hpp>

namespace gim::ecs::components::EngineState {
enum EngineState : uint32_t {
    Quitting = 0,
    Idle = 1,
    Running = 2,
};

class Component : public gim::ecs::IComponent {
  public:
    EngineState state = EngineState::Running;

    Component() = default;
    Component(const Component &) = default;
    Component(Component &&) = delete;
    auto operator=(const Component &) -> Component & = default;
    auto operator=(Component &&) -> Component & = delete;
    ~Component() override = default;
};
} // namespace gim::ecs::components::EngineState
