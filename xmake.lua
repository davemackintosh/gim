add_requires("vcpkg::doctest")
set_languages("c99", "c++20")
set_warnings("all")
-- Set the toolchain to Clang.
set_toolset("clang")
add_includedirs("include")

target("gim")
    set_kind("shared")
    add_files("src/*/**.cpp")
	remove_files("src/*/test_*.cpp")

	add_rules("mode.release", "mode.debug")
	if is_mode("release") then
		set_optimize("fastest")
	else
		set_rules("mode.debug", "mode.check", "mode.asan", "mode.tsan", "mode.ubsan", "mode.coverage")
		set_symbols("debug")
		set_optimize("none")
	end

target("test_gim")
	add_rules("mode.debug")
    set_kind("binary")
	set_rules("mode.debug", "mode.check", "mode.asan", "mode.tsan", "mode.ubsan", "mode.coverage")
    add_files("src/**/test_*.cpp")
	add_packages("vcpkg::doctest")
