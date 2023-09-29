#pragma once

#include <vector>

namespace gim::library::fs {
std::vector<char> readFile(const std::string &filename);
}