#pragma once

#include <algorithm>
#include <gim/ecs/engine/component_array.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace gim::ecs::components {
class Shader {
	enum ShaderType {
		COMPUTE,
		VERTEX,
		FRAGMENT,
	};

  public:
	template <typename T> void setData(T data);
};
} // namespace gim::ecs::components
