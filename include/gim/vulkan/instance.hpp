#pragma once

#include <SDL2/SDL.h>
#include <VkBootstrap.h>
#include <gim/engine.hpp>
#include <gim/library/fs.hpp>
#include <gim/library/glsl.hpp>
#include <gim/vulkan/utils.hpp>
#include <vk_mem_alloc.h>
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
    VmaAllocator allocator;

    Instance() {
        createInstance();
        createWindow();
        pickPhysicalDevice();
        createAllocator();
        createSwapchain();
        getQueues();
        createRenderPass();
    }

    auto createAllocator() -> void {
        VmaAllocatorCreateInfo allocatorInfo = {
            .physicalDevice = physicalDevice.physical_device,
            .device = device,
            .instance = instance};

        vmaCreateAllocator(&allocatorInfo, &allocator);
    }

    auto createSwapchain() -> void {
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

  private:
    vkb::PhysicalDevice physicalDevice;

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
        physicalDevice = phys_device_ret.value();
        vkb::DeviceBuilder device_builder{physicalDevice};
        auto device_ret = device_builder.build();
        if (!device_ret) {
            throw std::runtime_error(device_ret.error().message());
        }

        device = device_ret.value();
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

#pragma mark - pipelines

    auto createRenderPass() -> void {
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = swapchain.image_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &render_pass_info, nullptr,
                               &data.render_pass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
};

} // namespace gim::vulkan
