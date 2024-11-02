#pragma once

#include <gim/ecs/components/shader-base.hpp>
#include <gim/ecs/engine/component_array.hpp>
#include <gim/library/fs.hpp>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <utility>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace gim::ecs::components::Shader {

class TriangleVertexShaderData {
  public:
    std::vector<Vertex> vertices;
};

class TriangleBindings final
    : public gim::ecs::components::Shader::Bindings<TriangleVertexShaderData,
                                                    void *, void *> {

  public:
    explicit TriangleBindings(std::shared_ptr<TriangleVertexShaderData> data)
        : gim::ecs::components::Shader::Bindings<TriangleVertexShaderData,
                                                 void *, void *>(
              std::move(data)) {}

    TriangleBindings() = default;

    TriangleBindings(const TriangleBindings &) = default;
    TriangleBindings(TriangleBindings &&) = delete;
    auto operator=(const TriangleBindings &) -> TriangleBindings & = default;
    auto operator=(TriangleBindings &&) -> TriangleBindings & = delete;

    auto getBufferSize() -> unsigned long override {
        return sizeof(TriangleVertexShaderData) * vertData->vertices.size();
    }

    auto getVertexBindingDescriptionsForDevice()
        -> std::vector<VkVertexInputBindingDescription> override {
        return {
            {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        };
    }

    auto getVertexAttributesDescriptionsForDevice()
        -> std::vector<VkVertexInputAttributeDescription> override {
        return {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, position),
            },
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
        };
    }

    auto getFragmentBindingDescriptionsForDevice()
        -> std::vector<VkDescriptorSetLayoutBinding> override {
        return {};
    }

    auto getFragmentAttributesDescriptionsForDevice()
        -> std::vector<VkDescriptorSetLayoutBinding> override {
        return {};
    }

    ~TriangleBindings() = default;
};

class TriangleShader : public gim::ecs::components::Shader::Component {
  private:
    std::shared_ptr<TriangleBindings> bindings;

  public:
    explicit TriangleShader(const std::shared_ptr<TriangleBindings> &bindings)
        : Component(gim::library::fs::readFile("shaders/triangle.vert.spv"),
                    gim::library::fs::readFile("shaders/triangle.frag.spv")),
          bindings(bindings) {}

    auto getBindings() -> std::shared_ptr<TriangleBindings> { return bindings; }
};
} // namespace gim::ecs::components::Shader
