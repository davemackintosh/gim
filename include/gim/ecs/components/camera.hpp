#pragma once

#include <gim/ecs/engine/component_array.hpp>
#include <glm/vec3.hpp>

namespace gim::ecs::components::Camera {
enum Mode : uint32_t {
    FirstPerson = 0,
    RTS = 1,
};

class Component : public gim::ecs::IComponent {
  public:
    Mode mode = Mode::FirstPerson;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, -0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    float speed = 0.05f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float sensitivity = 0.1f;

    Component() = default;
    ~Component() override = default;
};
} // namespace gim::ecs::components::Camera
