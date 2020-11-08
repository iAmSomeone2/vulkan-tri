cmake_minimum_required(VERSION 3.15)
project(vulkan-tri VERSION 0.1.0)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLM REQUIRED glm)

add_subdirectory(deps)

add_subdirectory(shaders)

set(glfw_INCLUDES ${CMAKE_SOURCE_DIR}/deps/glfw/include/GLFW/)

add_subdirectory(src)
