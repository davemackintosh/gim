#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/glm.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <gim/vulkan/vulkan.hpp>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define SDLMustBeTrue(expr)                                                    \
	if (!(expr)) {                                                             \
		std::cerr << "SDL Error: " << SDL_GetError() << std::endl;             \
		exit(EXIT_FAILURE);                                                    \
	}

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugUtilsMessengerEXT *pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
								   VkDebugUtilsMessengerEXT callback,
								   const VkAllocationCallbacks *pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

gim::renderers::vulkan::VulkanApp::VulkanApp(
	std::vector<gim::ecs::components::Vertex> &vertices) {
	this->vertices = &vertices;
	initWindow();
	initVulkan();
}

auto gim::renderers::vulkan::VulkanApp::run() -> void {
	mainLoop();
	cleanup();
}

auto gim::renderers::vulkan::VulkanApp::initWindow() -> void {
	SDLMustBeTrue(SDL_Init(SDL_INIT_EVERYTHING) == 0);

	window = SDL_CreateWindow(
		"Vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	SDLMustBeTrue(window != nullptr);
}

auto gim::renderers::vulkan::VulkanApp::initVulkan() -> void {
	createInstance();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createVertexBuffer();
	createCommandBuffers();
	createSyncObjects();
}

auto gim::renderers::vulkan::VulkanApp::mainLoop() -> void {
	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return;
			} else if (event.type == SDL_WINDOWEVENT &&
					   event.window.event == SDL_WINDOWEVENT_RESIZED) {
				framebufferResized = true;
			}
		}

		drawFrame();
	}
	auto waitForFences =
		device->waitForFences(1, &inFlightFences[currentFrame], VK_TRUE,
							  std::numeric_limits<uint64_t>::max());

	if (waitForFences != vk::Result::eSuccess) {
		std::cout << "failed to wait for fences!" << std::endl;
	}

	device->waitIdle();
}

auto gim::renderers::vulkan::VulkanApp::cleanupSwapChain() -> void {
	for (auto framebuffer : swapChainFramebuffers) {
		device->destroyFramebuffer(framebuffer);
	}

	device->freeCommandBuffers(commandPool, commandBuffers);

	device->destroyPipeline(*graphicsPipeline);
	device->destroyPipelineLayout(pipelineLayout);
	device->destroyRenderPass(renderPass);

	for (auto imageView : swapChainImageViews) {
		device->destroyImageView(imageView);
	}

	device->destroySwapchainKHR(swapChain);
}

auto gim::renderers::vulkan::VulkanApp::cleanup() -> void {
	// NOTE: instance destruction is handled by UniqueInstance, same for
	// device

	// Wait for all frames to finish before destroying resources.
	device->waitIdle();
	cleanupSwapChain();

	device->destroyBuffer(vertexBuffer);
	device->freeMemory(vertexBufferMemory);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		device->destroySemaphore(renderFinishedSemaphores[i]);
		device->destroySemaphore(imageAvailableSemaphores[i]);
		device->destroyFence(inFlightFences[i]);
	}

	device->destroyCommandPool(commandPool);

	// surface is created by glfw, therefore not using a Unique handle
	instance->destroySurfaceKHR(surface);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(*instance, callback, nullptr);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
}

auto gim::renderers::vulkan::VulkanApp::recreateSwapChain() -> void {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		SDL_GetWindowSize(window, &width, &height);
		SDL_WaitEvent(nullptr);
	}

	device->waitIdle();

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandBuffers();
}

auto gim::renderers::vulkan::VulkanApp::createInstance() -> void {
	if (!checkValidationLayerSupport()) {
		throw std::runtime_error(
			"validation layers requested, but not available!");
	}

	auto appInfo = vk::ApplicationInfo(
		"Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine",
		VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	auto extensions = getRequiredExtensions();

	auto createInfo = vk::InstanceCreateInfo(
		vk::InstanceCreateFlags(), &appInfo, 0, nullptr, // enabled layers
		static_cast<uint32_t>(extensions.size()),
		extensions.data() // enabled extensions
	);

	if (enableValidationLayers) {
		auto debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT(
			vk::DebugUtilsMessengerCreateFlagsEXT(),
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			debugCallback, nullptr);

		createInfo.enabledLayerCount =
			static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.pNext = &debugCreateInfo;
	}

	try {
		instance = vk::createInstanceUnique(createInfo, nullptr);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create instance!");
	}
}

auto gim::renderers::vulkan::VulkanApp::createSurface() -> void {
	VkSurfaceKHR rawSurface;
	SDLMustBeTrue(
		SDL_Vulkan_CreateSurface(window, instance.get(), &rawSurface));
	surface = rawSurface;
}

auto gim::renderers::vulkan::VulkanApp::pickPhysicalDevice() -> void {
	auto devices = instance->enumeratePhysicalDevices();
	if (devices.empty()) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	for (const auto &maybeSuitableDevice : devices) {
		if (isDeviceSuitable(maybeSuitableDevice)) {
			physicalDevice = maybeSuitableDevice;
			break;
		}
	}

	if (!physicalDevice) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

auto gim::renderers::vulkan::VulkanApp::createLogicalDevice() -> void {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

	std ::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
											   indices.presentFamily.value()};

	float queuePriority = 1.0f;

	std::transform(
		uniqueQueueFamilies.begin(), uniqueQueueFamilies.end(),
		std::back_inserter(queueCreateInfos), [&](uint32_t queueFamily) {
			return vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(),
											 queueFamily, 1, &queuePriority);
		});

	auto deviceFeatures = vk::PhysicalDeviceFeatures();
	auto createInfo = vk::DeviceCreateInfo(
		vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfos.size()),
		queueCreateInfos.data());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount =
		static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount =
			static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	try {
		device = physicalDevice.createDeviceUnique(createInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create logical device!");
	}

	graphicsQueue = device->getQueue(indices.graphicsFamily.value(), 0);
	presentQueue = device->getQueue(indices.presentFamily.value(), 0);
}

auto gim::renderers::vulkan::VulkanApp::createSwapChain() -> void {
	SwapChainSupportDetails swapChainSupport =
		querySwapChainSupport(physicalDevice);

	vk::SurfaceFormatKHR surfaceFormat =
		chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode =
		chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo(
		vk::SwapchainCreateFlagsKHR(), surface, imageCount,
		surfaceFormat.format, surfaceFormat.colorSpace, extent,
		1, // imageArrayLayers
		vk::ImageUsageFlagBits::eColorAttachment);

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
									 indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

	try {
		swapChain = device->createSwapchainKHR(createInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create swap chain!");
	}

	swapChainImages = device->getSwapchainImagesKHR(swapChain);

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

auto gim::renderers::vulkan::VulkanApp::createImageViews() -> void {
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vk::ImageViewCreateInfo createInfo = {};
		createInfo.image = swapChainImages[i];
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;
		createInfo.subresourceRange.aspectMask =
			vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		try {
			swapChainImageViews[i] = device->createImageView(createInfo);
		} catch (vk::SystemError &err) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

auto gim::renderers::vulkan::VulkanApp::createRenderPass() -> void {
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	// dependency.srcAccessMask = 0;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
							   vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	try {
		renderPass = device->createRenderPass(renderPassInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create render pass!");
	}
}

auto gim::renderers::vulkan::VulkanApp::createGraphicsPipeline() -> void {
	auto vertShaderCode = readFile("shaders/default.vert.spv");
	auto fragShaderCode = readFile("shaders/default.frag.spv");

	auto vertShaderModule = createShaderModule(vertShaderCode);
	auto fragShaderModule = createShaderModule(fragShaderCode);

	vk::PipelineShaderStageCreateInfo shaderStages[] = {
		{vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex,
		 *vertShaderModule, "main"},
		{vk::PipelineShaderStageCreateFlags(),
		 vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main"}};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	auto bindingDescription =
		gim::ecs::components::Vertex::getBindingDescription();
	auto attributeDescriptions =
		gim::ecs::components::Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor = {{0, 0}, swapChainExtent};

	vk::PipelineViewportStateCreateInfo viewportState = {};
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;

	vk::PipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	vk::PipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	try {
		pipelineLayout = device->createPipelineLayout(pipelineLayoutInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;

	try {
		graphicsPipeline = std::make_shared<vk::Pipeline>(
			device->createGraphicsPipeline(nullptr, pipelineInfo).value);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

auto gim::renderers::vulkan::VulkanApp::createFrameBuffers() -> void {
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vk::ImageView attachments[] = {swapChainImageViews[i]};

		vk::FramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		try {
			swapChainFramebuffers[i] =
				device->createFramebuffer(framebufferInfo);
		} catch (vk::SystemError &err) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

auto gim::renderers::vulkan::VulkanApp::createCommandPool() -> void {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	vk::CommandPoolCreateInfo poolInfo = {};
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	try {
		commandPool = device->createCommandPool(poolInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create command pool!");
	}
}

auto gim::renderers::vulkan::VulkanApp::createVertexBuffer() -> void {
	vk::DeviceSize bufferSize = sizeof(vertices->at(0)) * vertices->size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				 vk::MemoryPropertyFlagBits::eHostVisible |
					 vk::MemoryPropertyFlagBits::eHostCoherent,
				 stagingBuffer, stagingBufferMemory);

	void *data = device->mapMemory(stagingBufferMemory, 0, bufferSize);
	memcpy(data, (void *)vertices->data(), (size_t)bufferSize);
	device->unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize,
				 vk::BufferUsageFlagBits::eTransferDst |
					 vk::BufferUsageFlagBits::eVertexBuffer,
				 vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer,
				 vertexBufferMemory);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	device->destroyBuffer(stagingBuffer);
	device->freeMemory(stagingBufferMemory);
}

auto gim::renderers::vulkan::VulkanApp::createBuffer(
	vk::DeviceSize size, vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties, vk::Buffer &buffer,
	vk::DeviceMemory &bufferMemory) -> void {
	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		buffer = device->createBuffer(bufferInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create buffer!");
	}

	vk::MemoryRequirements memRequirements =
		device->getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex =
		findMemoryType(memRequirements.memoryTypeBits, properties);

	try {
		bufferMemory = device->allocateMemory(allocInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	device->bindBufferMemory(buffer, bufferMemory, 0);
}

auto gim::renderers::vulkan::VulkanApp::copyBuffer(VkBuffer srcBuffer,
												   VkBuffer dstBuffer,
												   VkDeviceSize size) -> void {
	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer =
		device->allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	commandBuffer.begin(beginInfo);

	vk::BufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

	commandBuffer.end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	graphicsQueue.submit(submitInfo, nullptr);
	graphicsQueue.waitIdle();

	device->freeCommandBuffers(commandPool, commandBuffer);
}

auto gim::renderers::vulkan::VulkanApp::findMemoryType(
	uint32_t typeFilter, vk::MemoryPropertyFlags properties) -> uint32_t {
	vk::PhysicalDeviceMemoryProperties memProperties =
		physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) ==
				properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

auto gim::renderers::vulkan::VulkanApp::createCommandBuffers() -> void {
	commandBuffers.resize(swapChainFramebuffers.size());

	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	try {
		commandBuffers = device->allocateCommandBuffers(allocInfo);
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

		try {
			commandBuffers[i].begin(beginInfo);
		} catch (vk::SystemError &err) {
			throw std::runtime_error(
				"failed to begin recording command buffer!");
		}

		vk::RenderPassBeginInfo renderPassInfo = {

		};
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
		renderPassInfo.renderArea.extent = swapChainExtent;

		vk::ClearValue clearColor = {
			std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		commandBuffers[i].beginRenderPass(renderPassInfo,
										  vk::SubpassContents::eInline);

		commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics,
									   *graphicsPipeline);

		vk::Buffer vertexBuffers[] = {vertexBuffer};
		vk::DeviceSize offsets[] = {0};
		commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);

		commandBuffers[i].draw(static_cast<uint32_t>(vertices->size()), 1, 0,
							   0);

		commandBuffers[i].endRenderPass();

		try {
			commandBuffers[i].end();
		} catch (vk::SystemError &err) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

auto gim::renderers::vulkan::VulkanApp::createSyncObjects() -> void {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	try {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			imageAvailableSemaphores[i] = device->createSemaphore({});
			renderFinishedSemaphores[i] = device->createSemaphore({});
			inFlightFences[i] =
				device->createFence({vk::FenceCreateFlagBits::eSignaled});
		}
	} catch (vk::SystemError &err) {
		throw std::runtime_error(
			"failed to create synchronization objects for a frame!");
	}
}

auto gim::renderers::vulkan::VulkanApp::drawFrame() -> void {
	auto waitResult =
		device->waitForFences(1, &inFlightFences[currentFrame], VK_TRUE,
							  std::numeric_limits<uint64_t>::max());

	if (waitResult != vk::Result::eSuccess) {
		std::cout << "failed to wait for fences!" << std::endl;
	}

	uint32_t imageIndex;
	try {
		vk::ResultValue result = device->acquireNextImageKHR(
			swapChain, std::numeric_limits<uint64_t>::max(),
			imageAvailableSemaphores[currentFrame], nullptr);
		imageIndex = result.value;
	} catch (vk::OutOfDateKHRError &err) {
		recreateSwapChain();
		return;
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vk::SubmitInfo submitInfo = {};

	vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	vk::PipelineStageFlags waitStages[] = {
		vk::PipelineStageFlagBits::eColorAttachmentOutput};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	auto fenceReset = device->resetFences(1, &inFlightFences[currentFrame]);
	if (fenceReset != vk::Result::eSuccess) {
		std::cout << "failed to reset fences!" << std::endl;
	}

	try {
		graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);
		// Wait for the fence to signal that command buffer has finished.
		auto waitForFencesResult =
			device->waitForFences(1, &inFlightFences[currentFrame], VK_TRUE,
								  std::numeric_limits<uint64_t>::max());
		if (waitForFencesResult != vk::Result::eSuccess) {
			std::cout << "failed to wait for fences!" << std::endl;
		}
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	vk::SwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	vk::Result resultPresent;
	try {
		resultPresent = presentQueue.presentKHR(presentInfo);
	} catch (vk::OutOfDateKHRError &err) {
		resultPresent = vk::Result::eErrorOutOfDateKHR;
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	if (resultPresent == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
		return;
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto gim::renderers::vulkan::VulkanApp::createShaderModule(
	const std::vector<char> &code) -> vk::UniqueShaderModule {
	try {
		return device->createShaderModuleUnique(
			{vk::ShaderModuleCreateFlags(), code.size(),
			 reinterpret_cast<const uint32_t *>(code.data())});
	} catch (vk::SystemError &err) {
		throw std::runtime_error("failed to create shader module!");
	}
}

auto gim::renderers::vulkan::VulkanApp::chooseSwapSurfaceFormat(
	const std::vector<vk::SurfaceFormatKHR> &availableFormats)
	-> vk::SurfaceFormatKHR {
	if (availableFormats.size() == 1 &&
		availableFormats[0].format == vk::Format::eUndefined) {
		return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
	}

	// Find the available format using std::find_if.
	return *std::find_if(availableFormats.begin(), availableFormats.end(),
						 [](const vk::SurfaceFormatKHR &availableFormat) {
							 return availableFormat.format ==
										vk::Format::eB8G8R8A8Unorm &&
									availableFormat.colorSpace ==
										vk::ColorSpaceKHR::eSrgbNonlinear;
						 });
}

auto gim::renderers::vulkan::VulkanApp::chooseSwapPresentMode(
	const std::vector<vk::PresentModeKHR> &availablePresentModes)
	-> vk::PresentModeKHR {
	vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

	for (const auto &availablePresentMode : availablePresentModes) {
		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			return availablePresentMode;
		} else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

auto gim::renderers::vulkan::VulkanApp::chooseSwapExtent(
	const vk::SurfaceCapabilitiesKHR &capabilities) -> vk::Extent2D {
	if (capabilities.currentExtent.width !=
		std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		SDL_GetWindowSize(window, &width, &height);

		vk::Extent2D actualExtent = {static_cast<uint32_t>(width),
									 static_cast<uint32_t>(height)};

		actualExtent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

auto gim::renderers::vulkan::VulkanApp::querySwapChainSupport(
	const vk::PhysicalDevice &targetDevice) -> SwapChainSupportDetails {
	SwapChainSupportDetails details;
	details.capabilities = targetDevice.getSurfaceCapabilitiesKHR(surface);
	details.formats = targetDevice.getSurfaceFormatsKHR(surface);
	details.presentModes = targetDevice.getSurfacePresentModesKHR(surface);

	return details;
}

auto gim::renderers::vulkan::VulkanApp::isDeviceSuitable(
	const vk::PhysicalDevice &targetDevice) -> bool {
	QueueFamilyIndices indices = findQueueFamilies(targetDevice);

	bool extensionsSupported = checkDeviceExtensionSupport(targetDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport =
			querySwapChainSupport(targetDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() &&
							!swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

auto gim::renderers::vulkan::VulkanApp::checkDeviceExtensionSupport(
	const vk::PhysicalDevice &device) -> bool {
	std::set<std::string> requiredExtensions(deviceExtensions.begin(),
											 deviceExtensions.end());

	for (const auto &extension : device.enumerateDeviceExtensionProperties()) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

auto gim::renderers::vulkan::VulkanApp::findQueueFamilies(
	vk::PhysicalDevice targetPhysicalDevice) -> QueueFamilyIndices {
	QueueFamilyIndices indices;

	auto queueFamilies = targetPhysicalDevice.getQueueFamilyProperties();

	int i = 0;
	for (const auto &queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 &&
			queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphicsFamily = i;
		}

		if (queueFamily.queueCount > 0 &&
			targetPhysicalDevice.getSurfaceSupportKHR(i, surface)) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

auto gim::renderers::vulkan::VulkanApp::getRequiredExtensions()
	-> std::vector<const char *> {
	uint32_t sdlExtensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);
	std::vector<const char *> sdlExtensions(sdlExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount,
									 sdlExtensions.data());

	std::vector<const char *> extensions(sdlExtensions.begin(),
										 sdlExtensions.end());

	return extensions;
}

auto gim::renderers::vulkan::VulkanApp::checkValidationLayerSupport() -> bool {
	auto availableLayers = vk::enumerateInstanceLayerProperties();
	for (const char *layerName : validationLayers) {
		bool layerFound = false;

		for (const auto &layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

auto gim::renderers::vulkan::VulkanApp::readFile(const std::string &filename)
	-> std::vector<char> {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VKAPI_ATTR auto VKAPI_CALL gim::renderers::vulkan::VulkanApp::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
	-> VkBool32 {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
};
