#pragma once

#include <gim/ecs/engine/component_array.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace gim::ecs::components::Camera {
enum Mode : uint32_t {
    FirstPerson = 0,
    RTS = 1,
};

struct UBO {
  public:
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    UBO(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
        : projectionMatrix(projectionMatrix), viewMatrix(viewMatrix) {}
};

class Component : public gim::ecs::IComponent {
  public:
    Mode mode = Mode::FirstPerson;
    float FOV = glm::radians(45.0f);
    float aspectRatio = 1.f; // static_cast<float>(windowWidth) /
                             // static_cast<float>(windowHeight);
    float nearPlane = 0.1f;
    float farPlane = 1000.f;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, -0.f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.f);
    float speed = 0.05f;
    float yaw = -90.f;
    float pitch = 0.f;
    float sensitivity = 0.1f;

    Component() = default;
    ~Component() override = default;

    [[nodiscard]] auto getShaderUBO() -> std::shared_ptr<UBO> {
        return std::make_shared<UBO>(getViewMatrix(), getProjectionMatrix());
    }

  private:
    [[nodiscard]] auto getProjectionMatrix() const -> glm::mat4 {
        return glm::perspective(FOV, aspectRatio, nearPlane, farPlane);
    }

    [[nodiscard]] auto getViewMatrix() const -> glm::mat4 {
        return glm::lookAt(position, position + front, up);
    }

#pragma mark - Shaders.

  public:
    static auto getBufferSize() -> unsigned long { return sizeof(UBO); }
};
} // namespace gim::ecs::components::Camera
