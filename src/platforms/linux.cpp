#include <cstdlib>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/systems/vulkan.hpp>
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

	ecs->registerComponent<gim::ecs::components::Vertex>();
	ecs->registerSystem<gim::systems::renderers::vulkan::VulkanApp>();

	for (const auto &vertex : vertices) {
		auto entity = ecs->createEntity();
		ecs->addComponent<gim::ecs::components::Vertex>(
			entity, std::make_shared<gim::ecs::components::Vertex>(vertex));
	}

	try {
		while (true) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					break;
				}
			}

			ecs->update();
		}
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
