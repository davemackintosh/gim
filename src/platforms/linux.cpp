#include <SDL_video.h>
#include <cstdlib>
#include <gim/ecs/components/engine-state.hpp>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/systems/vulkan.hpp>
#include <memory>

int main() {
    auto ecs = std::make_shared<gim::ecs::ECS>();

    // Register all the components.
    ecs->registerComponent<gim::ecs::components::EngineState::Component>();
    ecs->registerComponent<gim::ecs::components::Vertex>();

    // Register all the systems.
    ecs->registerSystem<gim::ecs::systems::VulkanRendererSystem>();

    // Track internal state.
    auto renderDataEntity = ecs->createEntity();
    auto engineStateEntity = ecs->createEntity();
    auto engineState =
        std::make_shared<gim::ecs::components::EngineState::Component>();

    // Register the components.
    ecs->addComponent<gim::ecs::components::EngineState::Component>(
        engineStateEntity, engineState);

    while (engineState->state != gim::ecs::components::EngineState::Quitting) {
        ecs->update();
    }

    return EXIT_SUCCESS;
}
