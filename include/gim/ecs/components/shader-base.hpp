#pragma once

#include "gim/vulkan/instance.hpp"
#include <VkBootstrap.h>
#include <fmt/core.h>
#include <gim/ecs/engine/component_array.hpp>
#include <gim/library/glsl.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace gim::ecs::components::Shader {

#pragma mark - Nodes

class Node {
  public:
    virtual ~Node() {}
};

struct Vertex {
  public:
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
};
class VertexBuffer : public Node {
  public:
    std::vector<Vertex> vertices;
    explicit VertexBuffer(std::vector<Vertex> &vertices)
        : vertices(std::move(vertices)) {}
};

class Uniform : public Node {
  public:
    virtual ~Uniform(){};
};

template <typename Contents> class ShaderUniform : public Uniform {
  public:
    Contents contents;
    explicit ShaderUniform(Contents contents) : contents(contents) {}
    ~ShaderUniform(){};

  private:
    vk::Buffer uniformBuffer;
    VmaAllocation uniformBufferAllocation;
};

#pragma mark - Bindings

template <typename V, typename F = V, typename C = V> class Bindings {
  public:
    std::shared_ptr<V> vertData;
    std::shared_ptr<F> fragData;
    std::shared_ptr<C> compData;

    explicit Bindings(std::shared_ptr<V> vertData = nullptr,
                      std::shared_ptr<F> fragData = nullptr,
                      std::shared_ptr<C> compData = nullptr)
        : vertData(vertData), fragData(fragData), compData(compData) {}

    //////////////
    // Bindings //
    //////////////

    virtual auto getVertexBindingDescriptionsForDevice()
        -> std::vector<VkVertexInputBindingDescription> = 0;
    virtual auto getFragmentBindingDescriptionsForDevice()
        -> std::vector<VkDescriptorSetLayoutBinding> = 0;

    ////////////////
    // Attributes //
    ////////////////

    virtual auto getVertexAttributesDescriptionsForDevice()
        -> std::vector<VkVertexInputAttributeDescription> = 0;
    virtual auto getFragmentAttributesDescriptionsForDevice()
        -> std::vector<VkDescriptorSetLayoutBinding> = 0;

    /////////////
    // Buffers //
    /////////////

    virtual auto getBufferSize() -> unsigned long = 0;
};

#pragma mark - ECS

class Component : public gim::ecs::IComponent {
  private:
    VkShaderModule vertModule = VK_NULL_HANDLE;
    VkShaderModule fragModule = VK_NULL_HANDLE;
    VkShaderModule compModule = VK_NULL_HANDLE;
    std::vector<char> vertCode =
        gim::library::fs::readFile("shaders/triangle.vert.spv");
    std::vector<char> fragCode =
        gim::library::fs::readFile("shaders/triangle.frag.spv");
    std::vector<char> computeCode;

  public:
    Component() = default;

    Component(std::vector<char> vertStage, std::vector<char> fragStage)
        : vertCode(std::move(vertStage)), fragCode(std::move(fragStage)){};
    Component(std::vector<char> vertStage, std::vector<char> fragStage,
              std::vector<char> computeStage)
        : vertCode(std::move(vertStage)), fragCode(std::move(fragStage)),
          computeCode(std::move(computeStage)){};

    ~Component(){};

    static auto getShaderStage(VkShaderStageFlagBits stage,
                               VkShaderModule shaderModule)
        -> VkPipelineShaderStageCreateInfo {
        VkPipelineShaderStageCreateInfo stageInfo = {};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = stage;
        stageInfo.module = shaderModule;
        stageInfo.pName = "main";

        return stageInfo;
    };

    auto getVertexStageCreateInfo(const vkb::Device &device)
        -> std::optional<VkPipelineShaderStageCreateInfo> {
        vertModule =
            gim::library::glsl::createShaderModule(device.device, vertCode);

        if (vertModule == VK_NULL_HANDLE) {
            return std::nullopt;
        }

        return getShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertModule);
    }

    auto getFragmentStageCreateInfo(const vkb::Device &device)
        -> std::optional<VkPipelineShaderStageCreateInfo> {
        fragModule =
            gim::library::glsl::createShaderModule(device.device, fragCode);

        if (fragModule == VK_NULL_HANDLE) {
            return std::nullopt;
        }

        return getShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragModule);
    };

    auto getComputeStageCreateInfo(const vkb::Device &device)
        -> std::optional<VkPipelineShaderStageCreateInfo> {
        compModule =
            gim::library::glsl::createShaderModule(device.device, computeCode);

        if (compModule == VK_NULL_HANDLE) {
            return std::nullopt;
        }

        return getShaderStage(VK_SHADER_STAGE_COMPUTE_BIT, compModule);
    };

    auto getShaderStageCreateInfo(const vkb::Device &device)
        -> std::vector<VkPipelineShaderStageCreateInfo> {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        if (!vertCode.empty()) {
            auto vertStage = getVertexStageCreateInfo(device);
            if (vertStage) {
                shaderStages.push_back(*vertStage);
            }
        } else {
            fmt::print("no vertCode");
        }

        if (!fragCode.empty()) {
            auto fragStage = getFragmentStageCreateInfo(device);
            if (fragStage) {
                shaderStages.push_back(*fragStage);
            }
        } else {
            fmt::print("no fragCode");
        }

        if (!computeCode.empty()) {
            auto compStage = getComputeStageCreateInfo(device);
            if (compStage) {
                shaderStages.push_back(*compStage);
            }
        } else {
            fmt::print("no computeCode");
        }

        return shaderStages;
    }
};

#pragma mark - Builder.

class ShaderBuilder {
  private:
    std::optional<Component> shader;
    std::vector<Vertex> vertices;
    std::vector<char> vertCode;
    std::vector<char> fragCode;
    std::vector<char> computeCode;
    std::map<std::string, std::shared_ptr<Uniform>> shaderUniforms;

  public:
    auto getVertices() { return vertices; }
    auto setVertexSprv(std::vector<char> vertSprv) -> ShaderBuilder * {
        this->vertCode = std::move(vertSprv);
        return this;
    }

    auto setFragmentSprv(std::vector<char> fragSprv) -> ShaderBuilder * {
        this->fragCode = std::move(fragSprv);
        return this;
    }

    auto setComputeSprv(std::vector<char> compSprv) -> ShaderBuilder * {
        this->computeCode = std::move(compSprv);
        return this;
    }

    template <typename T>
    auto setUniform(const std::string &name, T uniform) -> ShaderBuilder * {
        auto unboundUniform = std::make_shared<ShaderUniform<T>>(uniform);
        this->shaderUniforms[name] = unboundUniform;
        return this;
    }

    auto setVertices(const std::vector<Vertex> &verticesData)
        -> ShaderBuilder * {
        this->vertices = verticesData;
        return this;
    }

    static auto getVertexBindingDescriptionsForDevice()
        -> std::vector<VkVertexInputBindingDescription> {
        return {
            {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        };
    }

    static auto getVertexAttributesDescriptionsForDevice()
        -> std::vector<VkVertexInputAttributeDescription> {
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

    static auto getFragmentBindingDescriptionsForDevice()
        -> std::vector<VkDescriptorSetLayoutBinding> {
        return {};
    }

    static auto getFragmentAttributesDescriptionsForDevice()
        -> std::vector<VkDescriptorSetLayoutBinding> {
        return {};
    }

    auto build() -> std::shared_ptr<Component> {
        auto builtShader = std::make_shared<Component>();

        for (const auto &uniform : this->shaderUniforms) {
            auto name = uniform.first;
            auto uniformValue = uniform.second;
        }

        return builtShader;
    }
};

} // namespace gim::ecs::components::Shader
