#pragma once

#include <gim/ecs/engine/component_array.hpp>
namespace gim::ecs::components::EngineState {
enum EngineState : uint32_t {
    Idle = 0,
    Running = 1,
};

class Component : public IComponent {
  public:
    EngineState state = EngineState::Idle;
    Component() = default;
    ~Component() override = default;
};
}
