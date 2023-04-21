#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace gim::renderers::vulkan {

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

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static vk::VertexInputBindingDescription getBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		return bindingDescription;
	}

	static std::array<vk::VertexInputAttributeDescription, 2>
	getAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 2>
			attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};
class VulkanApp {
  public:
	void run();

  private:
	SDL_Window *window{};

	vk::UniqueInstance instance;
	VkDebugUtilsMessengerEXT callback{};
	vk::SurfaceKHR surface;

	vk::PhysicalDevice physicalDevice;
	vk::UniqueDevice device;

	vk::Queue graphicsQueue;
	vk::Queue presentQueue;

	vk::SwapchainKHR swapChain;
	std::vector<vk::Image> swapChainImages;
	vk::Format swapChainImageFormat;
	vk::Extent2D swapChainExtent;
	std::vector<vk::ImageView> swapChainImageViews;
	std::vector<vk::Framebuffer> swapChainFramebuffers;

	vk::RenderPass renderPass;
	vk::PipelineLayout pipelineLayout;
	std::shared_ptr<vk::Pipeline> graphicsPipeline;

	vk::CommandPool commandPool;

	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;

	std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>>
		commandBuffers;

	std::vector<vk::Semaphore> imageAvailableSemaphores;
	std::vector<vk::Semaphore> renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences;
	size_t currentFrame = 0;

	bool framebufferResized = false;

	auto initWindow() -> void;
	auto initVulkan() -> void;
	auto mainLoop() -> void;
	auto cleanupSwapChain() -> void;
	auto cleanup() -> void;
	auto recreateSwapChain() -> void;
	auto createInstance() -> void;
	auto createSurface() -> void;
	auto pickPhysicalDevice() -> void;
	auto createLogicalDevice() -> void;
	auto createSwapChain() -> void;
	auto createImageViews() -> void;
	auto createRenderPass() -> void;
	auto createGraphicsPipeline() -> void;
	auto createFrameBuffers() -> void;
	auto createCommandPool() -> void;
	auto createVertexBuffer() -> void;
	auto createCommandBuffers() -> void;
	auto createSyncObjects() -> void;
	auto drawFrame() -> void;
	auto isDeviceSuitable(const vk::PhysicalDevice &targetDevice) -> bool;
	auto findQueueFamilies(vk::PhysicalDevice targetPhysicalDevice)
		-> QueueFamilyIndices;
	auto getRequiredExtensions() -> std::vector<const char *>;
	static auto checkDeviceExtensionSupport(const vk::PhysicalDevice &device)
		-> bool;
	static auto checkValidationLayerSupport() -> bool;
	static auto readFile(const std::string &filename) -> std::vector<char>;
	static auto chooseSwapPresentMode(
		const std::vector<vk::PresentModeKHR> &availablePresentModes)
		-> vk::PresentModeKHR;
	static auto chooseSwapSurfaceFormat(
		const std::vector<vk::SurfaceFormatKHR> &availableFormats)
		-> vk::SurfaceFormatKHR;
	static VKAPI_ATTR auto VKAPI_CALL
	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				  VkDebugUtilsMessageTypeFlagsEXT messageType,
				  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
				  void *pUserData) -> VkBool32;
	auto chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
		-> vk::Extent2D;
	auto querySwapChainSupport(const vk::PhysicalDevice &targetDevice)
		-> SwapChainSupportDetails;
	auto createShaderModule(const std::vector<char> &code)
		-> vk::UniqueShaderModule;
	auto createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
					  vk::MemoryPropertyFlags properties, vk::Buffer &buffer,
					  vk::DeviceMemory &bufferMemory) -> void;
	auto copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
		-> void;
	auto findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		-> uint32_t;
};

} // namespace gim::renderers::vulkan
