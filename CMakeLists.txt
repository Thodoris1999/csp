cmake_minimum_required(VERSION 3.20)
project(csp VERSION 0.1.0)

# Find required tools and libraries
find_program(GLSLC_EXECUTABLE glslc REQUIRED)
find_program(SPIRV_CROSS_EXECUTABLE spirv-cross REQUIRED)
find_package(Vulkan REQUIRED)

# Build the C++ code generator tool first so csp.cmake can reference it
add_executable(csp_generate src/generate_shader_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/spirv-reflect/spirv_reflect.c)
target_include_directories(csp_generate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/spirv-reflect
    ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(csp_generate PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)
target_compile_features(csp_generate PRIVATE cxx_std_17)

# Include the csp cmake module
include(cmake/csp.cmake)

enable_testing()
add_subdirectory(test)
