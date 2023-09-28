#pragma once

#include <SDL2/SDL.h>
#include <VkBootstrap.h>
#include <gim/engine.hpp>
#include <gim/vulkan/utils.hpp>

namespace gim::vulkan {
const int WIDTH = 800;
const int HEIGHT = 600;

class Instance {
  public:
    SDL_Window *window;
    vkb::Instance instance;
    VkSurfaceKHR surface;
    vkb::Device device;
    vkb::Swapchain swapchain;

    Instance() {
        createInstance();
        createWindow();
    }

  private:
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

    auto pickPhysicalDevice() -> void {
        vkb::PhysicalDeviceSelector phys_device_selector(instance);
        auto phys_device_ret =
            phys_device_selector.set_surface(surface).select();
        if (!phys_device_ret) {
            std::cout << phys_device_ret.error().message() << "\n";
        }
        vkb::PhysicalDevice physical_device = phys_device_ret.value();

        vkb::DeviceBuilder device_builder{physical_device};
        auto device_ret = device_builder.build();
        if (!device_ret) {
            std::cout << device_ret.error().message() << "\n";
        }
        device = device_ret.value();
    }
};

} // namespace gim::vulkan