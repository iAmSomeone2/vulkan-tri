find_package(fmt REQUIRED)

add_executable(vk_tri
        main.cpp
        TriangleApp.cpp TriangleApp.hpp)

target_include_directories(vk_tri PUBLIC SYSTEM
        ${Vulkan_INCLUDE_DIRS}
        ${glfw_INCLUDES}
        ${fmt_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS})

target_link_libraries(vk_tri
        ${Vulkan_LIBRARIES}
        glfw
        fmt::fmt
        ${GLM_LIBRARIES})

target_compile_features(vk_tri PUBLIC
        cxx_std_17
        cxx_auto_type
        cxx_constexpr
        cxx_nullptr
        cxx_range_for
        cxx_noexcept)

if (${BUILD_TESTING})
    include(CTest)
    add_subdirectory(test)
endif ()
