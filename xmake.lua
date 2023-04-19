add_requires("vcpkg::doctest", "vcpkg::vulkan-hpp")
add_requires("vulkansdk", "glslang", "spirv-tools", { system = true })
add_requires("vcpkg::sdl2 2.24.0", {
	configs = {
		features = {
			"x11",
			"vulkan",
			"wayland"
		}
	}
})
set_languages("c99", "c++20")
set_warnings("all")
set_toolset("clang")
if not is_mode("release") then
	-- Enable address sanitizer.
	add_cxflags("-fsanitize=address", "-ftrapv")
	add_ldflags("-fsanitize=address")
end
add_includedirs("include")

target("gim")
set_kind("binary")
set_symbols("debug")
set_optimize("none")

local platform = is_plat("windows") and "windows" or is_plat("linux") and "linux" or is_plat("macosx") and "macos" or
	"unknown"
local arch = is_arch("x86_64") and "x86_64" or is_arch("i386") and "i386" or "unknown"
local mode = is_mode("debug") and "debug" or is_mode("release") and "release" or "unknown"

-- Compile the GLSL to the same directory as the binary.
add_rules("utils.glsl2spv", { outputdir = string.format("$(buildir)/%s/%s/%s/shaders", platform, arch, mode) })
add_rules("mode.debug", "mode.asan", "mode.tsan", "mode.ubsan")

add_files("src/*/**.cpp")
remove_files("src/*/test_*.cpp", "src/*/platforms/*.cpp")
add_files(string.format("src/platforms/%s.cpp", is_plat("windows") and "windows" or "linux"))
add_files("shaders/*.vert", "shaders/*.frag", "shaders/*.comp")

add_packages("glslang", "spirv-tools", "vulkansdk", "vulkan-hpp", "vcpkg::sdl2")


target("test_gim")
set_kind("binary")
add_defines("TEST")
set_rules("mode.debug", "mode.check", "mode.asan", "mode.tsan", "mode.ubsan", "mode.coverage")
set_symbols("debug")
set_optimize("none")
add_files("src/**/test_*.cpp")
add_packages("vcpkg::doctest")
