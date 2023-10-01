#include <fstream>
#include <gim/library/fs.hpp>
#include <iostream>

namespace gim::library::fs {
std::string getFSPrefix() {
    char *val = getenv("FS_PREFIX");
    return val == nullptr ? std::string("./") : std::string(val);
}

std::vector<char> readFile(const std::string &filename) {
    auto prefix = getFSPrefix();
    std::cout << std::filesystem::current_path() << std::endl;
    std::cout << "PREFIX: " << prefix << std::endl;
    auto path = prefix + filename;
    std::cout << "PATH: " << path << std::endl;
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(file_size));

    file.close();

    return buffer;
};
} // namespace gim::library::fs