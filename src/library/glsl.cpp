#include <gim/library/glsl.hpp>
#include <vulkan/vulkan.hpp>

namespace gim::library::glsl {
auto createShaderModule(VkDevice targetDevice, const std::vector<char> &code)
    -> VkShaderModule {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(targetDevice, &create_info, nullptr,
                             &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE; // failed to create shader module
    }

    return shaderModule;
};
} // namespace gim::library::glsl