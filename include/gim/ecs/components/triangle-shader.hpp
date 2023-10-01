#pragma once

#include <gim/ecs/components/shader-base.hpp>
#include <gim/ecs/engine/component_array.hpp>
#include <gim/library/fs.hpp>
#include <vulkan/vulkan.hpp>

namespace gim::ecs::components::Shader {
class TriangleBindings final
    : public gim::ecs::components::Shader::Bindings<void *> {
  public:
    auto getBindingsForDevice()
        -> std::vector<VkVertexInputAttributeDescription> override {
        return {};
    }

    ~TriangleBindings() = default;
};

class TriangleShader
    : public gim::ecs::components::Shader::Component<TriangleBindings> {
  public:
    TriangleShader()
        : Component<TriangleBindings>(
              gim::library::fs::readFile("shaders/triangle.vert.spv"),
              gim::library::fs::readFile("shaders/triangle.frag.spv")) {}
};
} // namespace gim::ecs::components::Shader
