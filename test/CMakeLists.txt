csp_add_program(triangle
    VERT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.vert
    FRAG ${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.frag
)

add_executable(test_csp main.cpp)
target_compile_features(test_csp PRIVATE cxx_std_17)
csp_shader_dependency(test_csp PRIVATE triangle)

add_test(NAME csp_test COMMAND test_csp)
