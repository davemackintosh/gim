#pragma once

#include <gim/ecs/engine/component_array.hpp>

namespace ecs::components::renderer {
class Node {
  public:
    virtual void execute() = 0;
    virtual auto hasChildNodes() -> bool = 0;
    virtual auto getChildNodes() -> std::vector<std::shared_ptr<Node>> = 0;
};

/**
 * This class sets up a rendering pipeline for the engine,
 * it is not responsible for the setup of Vulkan itself but rather
 * the setup and encapsulation of the rendering pipeline.
 *
 * Also, this graph/trie is not responsible for the actual rendering
 * actually, it sets up the ability to have sub-graphs in which user
 * space graphs are created for the rendering pipeline.
 *
 *             ┌───────────────┐
 *             │   pipeline    │
 *             └───────┬───────┘
 *          ┌──────────▼───────────┐
 *          │ buffer  declarations │
 *          └──────────┬───────────┘
 *             ┌───────▼───────┐
 *             │  render pass  │
 *             └───────┬───────┘
 *         ▲───────────┴────────────┬───────────────────────┐
 * ┌───────▼───────┐       ┌────────▼────────┐     ┌────────▼────────┐
 * │ vertex shader │       │ fragment shader │     │ compute shader  │
 * └───────────────┘       └────────▲────────┘     └────────┬────────┘
 *                                  └───────────────────────┘
 */
class RenderingDAG : gim::ecs::IComponent {
  private:
    std::vector<std::shared_ptr<Node>> nodes;

  public:
    void addNode(std::shared_ptr<Node> &node) { nodes.push_back(node); }

    void execute() {
        for (const auto &node : nodes) {
            executeNode(node);
        }
    }

    void executeNode(const std::shared_ptr<Node> &node) {
        node->execute();

        if (node->hasChildNodes()) {
            for (auto &childNode : node->getChildNodes()) {
                executeNode(childNode);
            }
        }
    }
};
} // namespace ecs::components::renderer
