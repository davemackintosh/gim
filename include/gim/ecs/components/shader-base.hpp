#pragma once

#include "vulkan/vulkan.hpp"
#include <gim/ecs/engine/component_array.hpp>

namespace gim::ecs::components::Shader {
template <typename V, typename F = V, typename C = V> class Bindings {
  public:
    V vertData;
    F fragData;
    C compData;

    virtual ~Bindings() = 0;
    virtual auto getBindingsForDevice()
        -> std::vector<VkVertexInputAttributeDescription> = 0;
};

template <typename BindingType> class Component : public gim::ecs::IComponent {
  public:
    std::vector<char> vertStage;
    std::vector<char> fragStage;
    std::vector<char> computeStage;
    BindingType shaderData;

    Component(std::vector<char> vertStage, std::vector<char> fragStage)
        : vertStage(std::move(vertStage)), fragStage(std::move(fragStage)){};
    Component(std::vector<char> vertStage, std::vector<char> fragStage,
              std::vector<char> computeStage)
        : vertStage(std::move(vertStage)), fragStage(std::move(fragStage)),
          computeStage(std::move(computeStage)){};
};
} // namespace gim::ecs::components::Shader