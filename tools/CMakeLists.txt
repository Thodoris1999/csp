find_package(Vulkan REQUIRED)

add_executable(csp_generate generate_shader_info.cpp
    ${CMAKE_SOURCE_DIR}/extern/spirv-reflect/spirv_reflect.c)

target_include_directories(csp_generate PRIVATE
    ${CMAKE_SOURCE_DIR}/extern
    ${CMAKE_SOURCE_DIR}/extern/spirv-reflect
    ${Vulkan_INCLUDE_DIRS})

target_compile_definitions(csp_generate PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)

target_compile_features(csp_generate PRIVATE cxx_std_17)
