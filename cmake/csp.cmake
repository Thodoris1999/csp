# csp.cmake - Main cmake module for the csp graphics abstraction library
# Provides csp_add_program and csp_shader_dependency macros

# Root directory of the csp project (parent of the cmake/ directory)
get_filename_component(_CSP_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(_CSP_ROOT_DIR "${_CSP_CMAKE_DIR}" DIRECTORY)
set(_CSP_TEMPLATES_DIR "${_CSP_ROOT_DIR}/templates")

# Interface library that carries the public csp include path.
# csp_shader_dependency links targets against this so <csp/csp.hpp> is always findable.
if(NOT TARGET csp::csp)
    add_library(csp_public_headers INTERFACE)
    add_library(csp::csp ALIAS csp_public_headers)
    target_include_directories(csp_public_headers INTERFACE
        "${_CSP_ROOT_DIR}/include"
        ${Vulkan_INCLUDE_DIRS})
endif()

# -----------------------------------------------------------------------
# csp_add_program(program_name
#     VERT  shader.vert
#     FRAG  shader.frag
#     [COMP shader.comp]
#     [GEOM shader.geom]
#     [TESC shader.tesc]
#     [TESE shader.tese]
# )
#
# For each shader:
#   1. Compile GLSL -> SPIR-V  (glslc)
#   2. Transpile SPIR-V -> OpenGL GLSL  (spirv-cross)
# Then run csp_generate to produce .hpp/.cpp using SPIRV-Reflect + inja
#
# Sets global properties so csp_shader_dependency can find the outputs.
# -----------------------------------------------------------------------
macro(csp_add_program program_name)
    cmake_parse_arguments(_CSP "" "" "VERT;FRAG;COMP;GEOM;TESC;TESE" ${ARGN})

    # Output directory for this program
    set(_out_dir "${CMAKE_CURRENT_BINARY_DIR}/csp_generated/${program_name}")
    file(MAKE_DIRECTORY "${_out_dir}")

    set(_stage_names "")
    set(_all_spv_outputs "")
    set(_all_ogl_glsl_outputs "")

    # Process stages in a canonical pipeline order
    foreach(_kw VERT TESC TESE GEOM FRAG COMP)
        string(TOLOWER "${_kw}" _stage)
        foreach(_shader ${_CSP_${_kw}})
            get_filename_component(_shader_name "${_shader}" NAME_WE)
            get_filename_component(_shader_ext  "${_shader}" EXT)

            # Output file paths
            set(_spv_out "${_out_dir}/${_shader_name}.${_stage}.spv")
            set(_ogl_out "${_out_dir}/${_shader_name}.${_stage}.ogl.glsl")

            # Step 1: Compile GLSL -> SPIR-V
            add_custom_command(
                OUTPUT "${_spv_out}"
                COMMAND "${GLSLC_EXECUTABLE}"
                        -fshader-stage=${_stage}
                        "${_shader}"
                        -o "${_spv_out}"
                DEPENDS "${_shader}"
                COMMENT "csp: Compiling ${_shader_name}${_shader_ext} -> SPIR-V"
                VERBATIM
            )

            # Step 2: Transpile SPIR-V -> OpenGL-compatible GLSL
            add_custom_command(
                OUTPUT "${_ogl_out}"
                COMMAND "${SPIRV_CROSS_EXECUTABLE}"
                        --version 330
                        --no-es
                        --output "${_ogl_out}"
                        "${_spv_out}"
                DEPENDS "${_spv_out}"
                COMMENT "csp: Transpiling ${_shader_name}.${_stage}.spv -> OpenGL GLSL"
                VERBATIM
            )

            list(APPEND _all_spv_outputs      "${_spv_out}")
            list(APPEND _all_ogl_glsl_outputs "${_ogl_out}")
            list(APPEND _stage_names          "${_stage}")
        endforeach()
    endforeach()

    # Outputs from code generation
    set(_gen_hpp "${_out_dir}/${program_name}_shader_info.hpp")
    set(_gen_cpp "${_out_dir}/${program_name}_shader_info.cpp")

    # Build the stage:spv argument list for csp_generate
    set(_stage_spv_args "")
    list(LENGTH _shaders _num_shaders)
    math(EXPR _last_idx "${_num_shaders} - 1")
    foreach(_i RANGE 0 ${_last_idx})
        list(GET _stage_names    ${_i} _sn)
        list(GET _all_spv_outputs ${_i} _sp)
        list(APPEND _stage_spv_args "${_sn}:${_sp}")
    endforeach()

    # Step 3: Run code generator using csp_generate (SPIRV-Reflect + inja)
    add_custom_command(
        OUTPUT "${_gen_hpp}" "${_gen_cpp}"
        COMMAND $<TARGET_FILE:csp_generate>
                "${program_name}"
                "${_out_dir}"
                "${_CSP_TEMPLATES_DIR}"
                ${_stage_spv_args}
        DEPENDS csp_generate ${_all_spv_outputs}
        COMMENT "csp: Generating shader info for program '${program_name}'"
        VERBATIM
    )

    # Create a named target for the generation step
    set(_gen_target "csp_generate_${program_name}")
    add_custom_target(${_gen_target}
        DEPENDS
            ${_all_spv_outputs}
            ${_all_ogl_glsl_outputs}
            "${_gen_hpp}"
            "${_gen_cpp}"
    )

    # Store info in global properties for csp_shader_dependency
    set_property(GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_HPP"    "${_gen_hpp}")
    set_property(GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_CPP"    "${_gen_cpp}")
    set_property(GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_INCDIR" "${_out_dir}")
    set_property(GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_TARGET" "${_gen_target}")
endmacro()

# -----------------------------------------------------------------------
# csp_shader_dependency(target program_name)
#
# Wires the generated .hpp/.cpp into `target` and sets up dependencies.
# -----------------------------------------------------------------------
macro(csp_shader_dependency target program_name)
    get_property(_gen_hpp    GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_HPP")
    get_property(_gen_cpp    GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_CPP")
    get_property(_inc_dir    GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_INCDIR")
    get_property(_gen_target GLOBAL PROPERTY "CSP_PROGRAM_${program_name}_TARGET")

    if(NOT _gen_hpp)
        message(FATAL_ERROR "csp: Program '${program_name}' not found. Did you call csp_add_program(${program_name} ...) first?")
    endif()

    # Add generated sources to the target
    target_sources(${target} PRIVATE "${_gen_cpp}")

    # Add the output directory so the generated .hpp is findable
    target_include_directories(${target} PRIVATE "${_inc_dir}")

    # Pull in the public csp headers (<csp/csp.hpp>) transitively
    target_link_libraries(${target} PUBLIC csp::csp)

    # Make target depend on the generation custom target
    add_dependencies(${target} "${_gen_target}")
endmacro()
