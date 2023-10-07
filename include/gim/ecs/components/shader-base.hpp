#pragma once

#include <VkBootstrap.h>
#include <gim/ecs/engine/component_array.hpp>
#include <gim/library/glsl.hpp>
#include <vulkan/vulkan.hpp>

namespace gim::ecs::components::Shader {
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

class Component : public gim::ecs::IComponent {
  private:
    VkShaderModule vertModule = VK_NULL_HANDLE;
    VkShaderModule fragModule = VK_NULL_HANDLE;
    VkShaderModule compModule = VK_NULL_HANDLE;

  public:
    std::vector<char> vertCode;
    std::vector<char> fragCode;
    std::vector<char> computeCode;

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
        }

        if (!fragCode.empty()) {
            auto fragStage = getFragmentStageCreateInfo(device);
            if (fragStage) {
                shaderStages.push_back(*fragStage);
            }
        }

        if (!computeCode.empty()) {
            auto compStage = getComputeStageCreateInfo(device);
            if (compStage) {
                shaderStages.push_back(*compStage);
            }
        }

        return shaderStages;
    }
};
} // namespace gim::ecs::components::Shader
