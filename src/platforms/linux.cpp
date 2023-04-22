#include <cstdlib>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/vulkan/vulkan.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>

int main() {
	auto vertices = std::vector<gim::ecs::components::Vertex>(
		{{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		 {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		 {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}});
	auto ecs = std::make_shared<gim::ecs::ECS>();
	auto app = std::make_unique<gim::renderers::vulkan::VulkanApp>(vertices);

	try {
		app->run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
