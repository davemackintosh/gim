#include <cstdlib>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/components/engine-state.hpp>
#include <gim/ecs/ecs.hpp>
#include <memory>
#include <gim/ecs/systems/vulkan.hpp>

int main() {
    auto ecs = std::make_shared<gim::ecs::ECS>();

    // Register all the components.
    ecs->registerComponent<gim::ecs::components::EngineState::Component>();
    ecs->registerComponent<gim::ecs::components::Vertex>();

    // Register all the systems.
    ecs->registerSystem<gim::ecs::systems::VulkanRendererSystem>();

    // Track internal state.
    auto engineStateEntity = ecs->createEntity();
    auto engineState = std::make_shared<gim::ecs::components::EngineState::Component>();
    ecs->addComponent<gim::ecs::components::EngineState::Component>(engineStateEntity, engineState);

    // Start the engine.
    engineState->state = gim::ecs::components::EngineState::Running;

    while (engineState->state == gim::ecs::components::EngineState::Running) {
        ecs->update();
    }

    return EXIT_SUCCESS;
}
