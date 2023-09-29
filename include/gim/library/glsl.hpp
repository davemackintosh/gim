#pragma once

#include <gim/vulkan/init.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace gim::library::glsl {
auto createShaderModule(VkDevice targetDevice, const std::vector<char> &code)
    -> VkShaderModule;
}