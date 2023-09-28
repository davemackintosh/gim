#pragma once

#include <gim/ecs/engine/component_array.hpp>
namespace gim::ecs::components::EngineState {
enum EngineState : uint32_t {
    Quitting = 0,
    Idle = 1,
    Running = 2,
};

class Component : public IComponent {
  public:
    EngineState state = EngineState::Running;
    Component() = default;
    ~Component() override = default;
};
} // namespace gim::ecs::components::EngineState
