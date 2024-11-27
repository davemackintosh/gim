cmake_minimum_required(VERSION 3.22.1)
include(FetchContent)
include("./cmake/compileGLSL.cmake")

project(
  walk-to-utopia
  VERSION 0.1.0.0
  DESCRIPTION "Gim. I make gim."
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# DEPENDENCIES
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(VKB_INCLUDES VkBootstrap.h)
find_path(VMA_INCLUDES vk_mem_alloc.h)

# SOURCES
include("./cmake/memory-safety.cmake")
set(SOURCES ./src/platforms/linux.cpp ./src/ecs/systems/vulkan.cpp
            ./src/library/fs.cpp ./src/library/vma.cpp)
include_directories(./include ${VKB_INCLUDES} ${VMA_INCLUDES}
                    ${CMAKE_CURRENT_BINARY_DIR}/include)
configure_file(./include/gim/engine.hpp.in ./include/gim/engine.hpp)

add_executable(${PROJECT_NAME} ${SOURCES})
append_glsl_to_target(
  ${PROJECT_SOURCE_DIR}/shaders/voxel.comp
  ${PROJECT_SOURCE_DIR}/shaders/default.frag
  ${PROJECT_SOURCE_DIR}/shaders/default.vert
  ${PROJECT_SOURCE_DIR}/shaders/triangle.vert
  ${PROJECT_SOURCE_DIR}/shaders/triangle.frag)

# LINKER
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE glm::glm
          $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
          $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
          Vulkan::Vulkan
          vk-bootstrap::vk-bootstrap
          GPUOpen::VulkanMemoryAllocator
          fmt::fmt)
