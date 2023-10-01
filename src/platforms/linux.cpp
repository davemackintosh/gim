#include <SDL_video.h>
#include <cstdlib>
#include <gim/ecs/components/engine-state.hpp>
#include <gim/ecs/components/shader-base.hpp>
#include <gim/ecs/components/triangle-shader.hpp>
#include <gim/ecs/components/vertex.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/systems/vulkan.hpp>
#include <memory>

struct VertexData {};

int main() {
    auto ecs = std::make_shared<gim::ecs::ECS>();

    // Register all the components.
    ecs->registerComponent<gim::ecs::components::EngineState::Component>();
    ecs->registerComponent<gim::ecs::components::Vertex>();
    ecs->registerComponent<
        gim::ecs::components::Shader::Component<VertexData>>();
    ecs->registerComponent<gim::ecs::components::Shader::TriangleShader>();

    // Register all the systems.
    ecs->registerSystem<gim::ecs::systems::VulkanRendererSystem>();

    // Track internal state.
    auto engineStateEntity = ecs->createEntity();
    auto engineState =
        std::make_shared<gim::ecs::components::EngineState::Component>();

    auto triangleShaderEntity = ecs->createEntity();
    auto triangleShader =
        std::make_shared<gim::ecs::components::Shader::TriangleShader>();

    // Register the components.
    ecs->addComponent<gim::ecs::components::EngineState::Component>(
        engineStateEntity, engineState);
    ecs->addComponent<gim::ecs::components::Shader::TriangleShader>(
        triangleShaderEntity, triangleShader);

    while (engineState->state != gim::ecs::components::EngineState::Quitting) {
        ecs->update();
    }

    return EXIT_SUCCESS;
}
