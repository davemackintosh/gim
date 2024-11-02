#pragma once

#include <gim/ecs/engine/component_array.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

const static auto FOV_DEFAULT = 45.0F;
const static auto NEAR_PLANE_DEFAULT = 0.1F;
const static auto FAR_PLANE_DEFAULT = 1000.F;
const static auto SPEED_DEFAULT = 0.05F;
const static auto YAW_DEFAULT = -90.F;
const static auto PITCH_DEFAULT = 0.F;
const static auto SENSITIVITY_DEFAULT = 0.1F;

namespace gim::ecs::components::Camera {
enum Mode : uint32_t {
    FirstPerson = 0,
    RTS = 1,
};

struct UBO {
  public:
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    UBO(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) // NOLINT
        : projectionMatrix(projectionMatrix), viewMatrix(viewMatrix) {}
};

class Component : public gim::ecs::IComponent {
  public:
    Mode mode = Mode::FirstPerson;
    float FOV = glm::radians(FOV_DEFAULT);
    float aspectRatio = 1.F; // static_cast<float>(windowWidth) /
                             // static_cast<float>(windowHeight);
    float nearPlane = NEAR_PLANE_DEFAULT;
    float farPlane = FAR_PLANE_DEFAULT;
    glm::vec3 position = glm::vec3(0.0F, 0.0F, -0.F);
    glm::vec3 front = glm::vec3(0.0F, 0.0F, -1.F);
    glm::vec3 up = glm::vec3(0.0F, 1.0F, 0.F);
    float speed = SPEED_DEFAULT;
    float yaw = YAW_DEFAULT;
    float pitch = PITCH_DEFAULT;
    float sensitivity = SENSITIVITY_DEFAULT;

    Component() = default;
    Component(const Component &) = default;
    Component(Component &&) = delete;
    auto operator=(const Component &) -> Component & = default;
    auto operator=(Component &&) -> Component & = delete;
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
