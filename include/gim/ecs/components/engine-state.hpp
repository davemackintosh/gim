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

};
} // namespace gim::ecs::components::EngineState
