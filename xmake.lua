add_requires("vcpkg::doctest")
set_languages("c99", "c++20")
set_warnings("all")
-- Set the toolchain to Clang.
set_toolset("clang")
if not is_mode("release") then
	-- Enable address sanitizer.
	add_cxflags("-fsanitize=address", "-ftrapv")
	add_ldflags("-fsanitize=address")
end
add_includedirs("include")

target("gim")
set_kind("shared")
add_files("src/*/**.cpp")
remove_files("src/*/test_*.cpp")

if is_mode("release") then
	set_symbols("hidden")
	set_optimize("fastest")
	set_strip("all")
else
	set_rules("mode.debug", "mode.check", "mode.asan", "mode.tsan", "mode.ubsan", "mode.coverage")
	set_symbols("debug")
	set_optimize("none")
end

target("test_gim")
set_kind("binary")
add_defines("TEST")
set_rules("mode.debug", "mode.check", "mode.asan", "mode.tsan", "mode.ubsan", "mode.coverage")
set_symbols("debug")
set_optimize("none")
add_files("src/**/test_*.cpp")
add_packages("vcpkg::doctest")
