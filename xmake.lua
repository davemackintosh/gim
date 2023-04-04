add_requires("vcpkg::doctest")
set_languages("c99", "c++20")
add_includedirs("include")

target("gim")
    set_kind("shared")
    add_files("src/*/**.cpp")
	remove_files("src/*/test_*.cpp")

	add_rules("mode.release", "mode.debug")
	if is_mode("release") then
		set_optimize("fastest")
	end

target("test_gim")
    set_kind("binary")
	add_defines("TEST")
	add_deps("gim")
    add_files("src/**/test_*.cpp")
	add_packages("vcpkg::doctest")
