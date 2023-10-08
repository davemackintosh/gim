#include <SDL_video.h>
#include <cstdlib>
#include <gim/ecs/components/camera.hpp>
#include <gim/ecs/components/engine-state.hpp>
#include <gim/ecs/components/shader-base.hpp>
#include <gim/ecs/components/triangle-shader.hpp>
#include <gim/ecs/ecs.hpp>
#include <gim/ecs/systems/vulkan.hpp>
#include <glm/fwd.hpp>
#include <memory>

struct VertexData {};

int main() {
    auto ecs = std::make_shared<gim::ecs::ECS>();

#pragma mark - Register components

    // Register all the components.
    ecs->registerComponent<gim::ecs::components::EngineState::Component>();
    ecs->registerComponent<gim::ecs::components::Shader::TriangleShader>();
    ecs->registerComponent<gim::ecs::components::Camera::Component>();

#pragma mark - Systems
    // Register all the systems.
    ecs->registerSystem<gim::ecs::systems::VulkanRendererSystem>();

#pragma mark - Game content

    // Track internal state.
    auto cameraEntity = ecs->createEntity();
    auto engineStateEntity = ecs->createEntity();
    auto engineState =
        std::make_shared<gim::ecs::components::EngineState::Component>();
    auto camera = std::make_shared<gim::ecs::components::Camera::Component>();

#pragma mark - Shaders

    auto triangleShaderEntity = ecs->createEntity();
    auto triangleBindings =
        std::make_shared<gim::ecs::components::Shader::TriangleBindings>(
            std::make_shared<
                gim::ecs::components::Shader::TriangleVertexShaderData>(
                gim::ecs::components::Shader::TriangleVertexShaderData{
                    .vertices =
                        std::vector<gim::ecs::components::Shader::Vertex>{
                            {
                                .position = glm::vec3{1.f, 1.f, 0.f},
                                .color = glm::vec4(1.f, 0.f, 0.f, 1.f),
                            },
                            {
                                .position = glm::vec3{-1.f, 1.f, 0.f},
                                .color = glm::vec4(0.f, 1.f, 0.f, 1.f),
                            },
                            {
                                .position = glm::vec3{0.f, -1.f, 0.f},
                                .color = glm::vec4(0.f, 0.f, 1.f, 1.f),
                            },
                        },
                }));
    auto triangleShader =
        std::make_shared<gim::ecs::components::Shader::TriangleShader>(
            triangleBindings);

#pragma mark - Add components

    // Register the components.
    ecs->addComponent(cameraEntity, camera);
    ecs->addComponent(engineStateEntity, engineState);
    ecs->addComponent(triangleShaderEntity, triangleShader);

#pragma mark - Run the game.

    while (engineState->state != gim::ecs::components::EngineState::Quitting) {
        ecs->update();
    }

    return EXIT_SUCCESS;
}
