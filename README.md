# csp — Cross-API Shader Program

csp is a CMake-integrated code generation library that gives graphics engines a
single source of truth for shader programs while allowing them to support multiple backends.
You write your shaders once in Vulkan-compatible GLSL. csp compiles them to
SPIR-V, transpiles to OpenGL-compatible GLSL, and uses SPIR-V reflection to
generate C++ tables that the engine can use to create the render API structs.

## System dependencies

- glslc
- spirv-cross
- Vulkan headers

On Ubuntu/Debian:

```sh
# Vulkan SDK provides glslc
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc \
  | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list \
  https://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
sudo apt-get update && sudo apt-get install -y vulkan-sdk

# spirv-cross
sudo apt-get install -y spirv-cross
```

## CMake interface

### `csp_add_program`

```cmake
csp_add_program(<program_name>
    VERT  <vertex_shader>
    FRAG  <fragment_shader>
    [COMP <compute_shader>]
    [GEOM <geometry_shader>]
    [TESC <tessellation_control_shader>]
    [TESE <tessellation_evaluation_shader>]
)
```

Registers a shader program. For each shader it:

1. Compiles GLSL → SPIR-V with `glslc`
2. Transpiles SPIR-V → OpenGL 3.3 GLSL with `spirv-cross`
3. Runs SPIR-V reflection and generates `<program_name>_shader_info.hpp/.cpp`

Each keyword may be repeated to supply multiple shaders for the same stage.
Stages are processed in canonical pipeline order (VERT → TESC → TESE → GEOM →
FRAG → COMP) regardless of the order they appear in the call.

### `csp_shader_dependency`

```cmake
csp_shader_dependency(<target> <program_name>)
```

Wires the generated `.hpp`/`.cpp` into `<target>`:
- Adds the generated `.cpp` as a private source
- Adds the generated output directory to the target's include path
- Links `csp::csp` (public csp headers) transitively

## Minimal example

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(my_app)

add_subdirectory(csp)
include(csp/cmake/csp.cmake)

csp_add_program(triangle
    VERT shaders/triangle.vert
    FRAG shaders/triangle.frag
)

add_executable(my_app main.cpp)
target_compile_features(my_app PRIVATE cxx_std_17)
csp_shader_dependency(my_app triangle)
```

```cpp
// main.cpp
#include <vulkan/vulkan.h>
#include <GL/gl.h>
#include "triangle_shader_info.hpp"  // generated

void setup_vulkan_pipeline(VkDevice device, VkPipelineLayout* layout) {
    // Use the generated table to create a push constant range.
    // stage_flags is a VkShaderStageFlagBits bitmask — cast directly.
    const auto& pc = triangle_ShaderInfo::csp_vk_push_constant_info[CSP_UNIFORM_TRIANGLE_PC];
    VkPushConstantRange range{ static_cast<VkShaderStageFlags>(pc.stage_flags), pc.offset, pc.size };
    // ... vkCreatePipelineLayout(device, ...) ...

    // Load SPIR-V from the filename recorded at build time.
    for (uint32_t i = 0; i < triangle_descriptor.source_count; ++i) {
        const auto& src = triangle_descriptor.vk_sources[i];
        // src.stage is csp::ShaderStage — map to VkShaderStageFlagBits as needed
        const char* spv_file = src.filename.data();
        // ... load and create VkShaderModule from spv_file ...
    }
}

void setup_opengl_program(GLuint program) {
    // Bind the uniform block using the generated name.
    const char* block_name = triangle_ShaderInfo::csp_ogl_uniform_info[CSP_UNIFORM_TRIANGLE_PC].name.data();
    GLuint block_index = glGetUniformBlockIndex(program, block_name);
    glUniformBlockBinding(program, block_index, CSP_UNIFORM_TRIANGLE_PC);

    // Load OpenGL GLSL from the filename recorded at build time.
    for (uint32_t i = 0; i < triangle_descriptor.source_count; ++i) {
        const auto& src = triangle_descriptor.ogl_sources[i];
        // src.stage is csp::ShaderStage — map to GLenum for glCreateShader as needed
        const char* glsl_file = src.filename.data();
        // ... load and compile glsl_file as a GL shader ...
    }
}
```

## Generated API

For a program named `triangle`, csp generates `triangle_shader_info.hpp` and
`triangle_shader_info.cpp` that provide:

### Uniform index macros

```cpp
#define CSP_UNIFORM_TRIANGLE_<NAME> <index>
```

Each index is valid for both the Vulkan and OpenGL tables simultaneously.

### `triangle_ShaderInfo` (static tables)

```cpp
struct triangle_ShaderInfo {
    // Vulkan push constant ranges — feed directly to vkCmdPushConstants /
    // VkPipelineLayoutCreateInfo. Non-push-constant entries have size = 0.
    // stage_flags is a raw VkShaderStageFlagBits bitmask.
    static constexpr csp::VkPushConstantEntry csp_vk_push_constant_info[];

    // OpenGL uniform names — pass to glGetUniformBlockIndex (UBOs / push
    // constant blocks) or glGetUniformLocation (samplers).
    static constexpr csp::OglUniformEntry csp_ogl_uniform_info[];

    // OpenGL transpiled shader filenames and stages, one per stage in pipeline order.
    static constexpr csp::ShaderSource csp_ogl_sources[];

    // Vulkan SPIR-V shader filenames and stages, one per stage in pipeline order.
    static constexpr csp::ShaderSource csp_vk_sources[];
};
```

### `triangle_descriptor` (`csp::ProgramDescriptor`)

A global `const csp::ProgramDescriptor` (see public API below) that bundles pointers to all four
tables together with their counts. Pass this to a backend that should not
include the program-specific header directly.

## Public stable API (`include/csp/csp.hpp`)
csp provides a header meant to be included by rendering engines. It has no dependency on Vulkan or OpenGL headers. Engines receive a `ProgramDescriptor` from the client application and access the information they need.

```cpp
enum class ShaderStage { Vertex, TessControl, TessEval, Geometry, Fragment, Compute };

struct VkPushConstantEntry {
    uint32_t offset;
    uint32_t size;
    uint32_t stage_flags; // VkShaderStageFlagBits bitmask — cast to VkShaderStageFlags
};

struct OglUniformEntry { std::string_view name; };

struct ShaderSource {
    ShaderStage      stage;
    std::string_view filename;
};

struct ProgramDescriptor {
    const VkPushConstantEntry* vk_push_constants;
    const OglUniformEntry*     ogl_uniforms;
    uint32_t                   uniform_count;
    const ShaderSource*        ogl_sources;
    const ShaderSource*        vk_sources;
    uint32_t                   source_count;
};
```
