#include <cstdlib>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/ecs.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <gim/ecs/systems/vulkan.hpp>

int main() {
    auto ecs = std::make_shared<gim::ecs::ECS>();
    auto verticesComponents = std::vector<gim::ecs::components::Vertex>(
        {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
         {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
         {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}});

    ecs->registerComponent<gim::ecs::components::Vertex>();
    ecs->registerSystem<gim::ecs::systems::VulkanRendererSystem>();

    try {
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
