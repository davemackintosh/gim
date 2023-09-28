#pragma once

#include <SDL2/SDL.h>
#include <VkBootstrap.h>
#include <gim/engine.hpp>
#include <gim/vulkan/utils.hpp>
#include <vulkan/vulkan.hpp>

namespace gim::vulkan {
const int WIDTH = 800;
const int HEIGHT = 600;

struct RenderData {
    VkQueue graphics_queue;
    VkQueue present_queue;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::vector<VkFramebuffer> framebuffers;

    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphore;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> image_in_flight;
    size_t current_frame = 0;
};
class Instance {
  public:
    SDL_Window *window;
    vkb::Instance instance;
    VkSurfaceKHR surface;
    vkb::Device device;
    vkb::Swapchain swapchain;
    RenderData data;

    Instance() {
        createInstance();
        createWindow();
        pickPhysicalDevice();
        getQueues();
    }

  private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

#pragma mark - Instance and window

    auto createInstance() -> void {
        vkb::InstanceBuilder instanceBuilder;
        auto builderResult = instanceBuilder.set_app_name(ENGINE_NAME)
                                 .request_validation_layers(true)
                                 .use_default_debug_messenger()
                                 .require_api_version(1, 2, 0)
                                 .build();

        instance = builderResult.value();
    }

    auto createWindow() -> void {
        SDLMustBeTrue(SDL_Init(SDL_INIT_EVERYTHING) == 0);

        window = SDL_CreateWindow(ENGINE_NAME, SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                  SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                      SDL_WINDOW_SHOWN);
        SDLMustBeTrue(window != nullptr);

        SDLMustBeTrue(SDL_Vulkan_CreateSurface(window, instance, &surface));
    }

#pragma mark - devices

    auto pickPhysicalDevice() -> void {
        vkb::PhysicalDeviceSelector phys_device_selector(instance);
        auto phys_device_ret =
            phys_device_selector.set_surface(surface).select();
        if (!phys_device_ret) {
            throw std::runtime_error(phys_device_ret.error().message());
        }
        vkb::PhysicalDevice physical_device = phys_device_ret.value();
        vkb::DeviceBuilder device_builder{physical_device};
        auto device_ret = device_builder.build();
        if (!device_ret) {
            throw std::runtime_error(device_ret.error().message());
        }

        device = device_ret.value();
    }

    auto createSwapchain() {

        vkb::SwapchainBuilder swapchain_builder{device};
        auto swap_ret = swapchain_builder.set_old_swapchain(swapchain).build();
        if (!swap_ret) {
            std::cout << swap_ret.error().message() << " "
                      << swap_ret.vk_result() << "\n";
            throw std::runtime_error(swap_ret.error().message());
        }
        vkb::destroy_swapchain(swapchain);
        swapchain = swap_ret.value();
    }

    auto getQueues() -> void {
        auto gq = device.get_queue(vkb::QueueType::graphics);
        if (!gq.has_value()) {
            throw std::runtime_error(gq.error().message());
        }
        data.graphics_queue = gq.value();

        auto pq = device.get_queue(vkb::QueueType::present);
        if (!pq.has_value()) {
            throw std::runtime_error(pq.error().message());
        }
        data.present_queue = pq.value();
    }
};

} // namespace gim::vulkan