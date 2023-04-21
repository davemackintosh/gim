#include <cstdlib>
#include <gim/vulkan/vulkan.hpp>
#include <iostream>
#include <ostream>
#include <stdexcept>

int main() {
	gim::renderers::vulkan::VulkanApp app;

	try {
		app.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
