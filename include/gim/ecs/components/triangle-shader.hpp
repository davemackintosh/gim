#pragma once

#include <gim/ecs/components/shader-base.hpp>
#include <gim/ecs/engine/component_array.hpp>
#include <gim/library/fs.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.hpp>

namespace gim::ecs::components::Shader {
class TriangleFragmentShader {
  public:
    std::vector<glm::vec4> colors;
};

class TriangleBindings final
    : public gim::ecs::components::Shader::Bindings<void *,
                                                    TriangleFragmentShader *> {
  public:
    auto getVertexBindingsForDevice()
        -> std::vector<VkVertexInputAttributeDescription> override {
        return {};
    }
    auto getFragmentBindingsForDevice()
        -> std::vector<VkDescriptorSetLayoutBinding> override {
        return std::vector<VkDescriptorSetLayoutBinding>(
            {{.binding = 0,
              .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              .descriptorCount = 1,
              .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT}});
    }

    ~TriangleBindings() = default;
};

class TriangleShader
    : public gim::ecs::components::Shader::Component<TriangleBindings> {
  public:
    TriangleBindings bindings;

    explicit TriangleShader(TriangleBindings &bindings)
        : Component<TriangleBindings>(
              gim::library::fs::readFile("shaders/triangle.vert.spv"),
              gim::library::fs::readFile("shaders/triangle.frag.spv")) {
        this->bindings = bindings;
    }
};
} // namespace gim::ecs::components::Shader
