#include <cstdio>
#include <cstdlib>
#include <csp/csp.hpp>
#include "triangle_shader_info.hpp"

int main() {
    bool all_ok = true;

    // Push constant member "pc.transform" is at index 0.
    // CSP_UNIFORM_TRIANGLE_PC_TRANSFORM == 0
    if (CSP_UNIFORM_TRIANGLE_PC_TRANSFORM != 0) {
        std::fprintf(stderr,
            "FAIL: CSP_UNIFORM_TRIANGLE_PC_TRANSFORM expected 0, got %d\n",
            CSP_UNIFORM_TRIANGLE_PC_TRANSFORM);
        all_ok = false;
    }

    // pc.transform is a mat4 (64 bytes, offset 0).
    constexpr uint32_t expected_size = 64u;
    constexpr uint32_t actual_size =
        triangle_ShaderInfo::csp_push_constant_info[CSP_UNIFORM_TRIANGLE_PC_TRANSFORM].size;

    if (actual_size != expected_size) {
        std::fprintf(stderr,
            "FAIL: csp_push_constant_info[PC_TRANSFORM].size expected %u, got %u\n",
            expected_size, actual_size);
        all_ok = false;
    }

    constexpr uint32_t actual_offset =
        triangle_ShaderInfo::csp_push_constant_info[CSP_UNIFORM_TRIANGLE_PC_TRANSFORM].offset;

    if (actual_offset != 0u) {
        std::fprintf(stderr,
            "FAIL: csp_push_constant_info[PC_TRANSFORM].offset expected 0, got %u\n",
            actual_offset);
        all_ok = false;
    }

    // Push constant is vertex-only: VK_SHADER_STAGE_VERTEX_BIT = 0x00000001.
    constexpr uint32_t pc_flags =
        triangle_ShaderInfo::csp_push_constant_info[CSP_UNIFORM_TRIANGLE_PC_TRANSFORM].stage_flags;

    if (pc_flags != 0x00000001u) {
        std::fprintf(stderr,
            "FAIL: push constant stage_flags expected 0x00000001 (VK_SHADER_STAGE_VERTEX_BIT), got 0x%08X\n",
            pc_flags);
        all_ok = false;
    }

    // Name is "pc.transform" for use with glGetUniformLocation.
    constexpr std::string_view pc_name =
        triangle_ShaderInfo::csp_push_constant_info[CSP_UNIFORM_TRIANGLE_PC_TRANSFORM].name;

    if (pc_name != "pc.transform") {
        std::fprintf(stderr,
            "FAIL: csp_push_constant_info[PC_TRANSFORM].name expected \"pc.transform\", got \"%.*s\"\n",
            static_cast<int>(pc_name.size()), pc_name.data());
        all_ok = false;
    }

    // OGL source filenames and stages (vert then frag, canonical pipeline order).
    constexpr std::string_view ogl_vert = triangle_ShaderInfo::csp_ogl_sources[0].filename;
    constexpr std::string_view ogl_frag = triangle_ShaderInfo::csp_ogl_sources[1].filename;

    if (ogl_vert != "triangle.vert.ogl.glsl") {
        std::fprintf(stderr,
            "FAIL: csp_ogl_sources[0].filename expected \"triangle.vert.ogl.glsl\", got \"%.*s\"\n",
            static_cast<int>(ogl_vert.size()), ogl_vert.data());
        all_ok = false;
    }

    if (ogl_frag != "triangle.frag.ogl.glsl") {
        std::fprintf(stderr,
            "FAIL: csp_ogl_sources[1].filename expected \"triangle.frag.ogl.glsl\", got \"%.*s\"\n",
            static_cast<int>(ogl_frag.size()), ogl_frag.data());
        all_ok = false;
    }

    if (triangle_ShaderInfo::csp_ogl_sources[0].stage != csp::ShaderStage::Vertex) {
        std::fprintf(stderr, "FAIL: csp_ogl_sources[0].stage is not ShaderStage::Vertex\n");
        all_ok = false;
    }

    if (triangle_ShaderInfo::csp_ogl_sources[1].stage != csp::ShaderStage::Fragment) {
        std::fprintf(stderr, "FAIL: csp_ogl_sources[1].stage is not ShaderStage::Fragment\n");
        all_ok = false;
    }

    // Vulkan source filenames and stages.
    constexpr std::string_view vk_vert_fn = triangle_ShaderInfo::csp_vk_sources[0].filename;
    constexpr std::string_view vk_frag_fn = triangle_ShaderInfo::csp_vk_sources[1].filename;

    if (vk_vert_fn != "triangle.vert.spv") {
        std::fprintf(stderr,
            "FAIL: csp_vk_sources[0].filename expected \"triangle.vert.spv\", got \"%.*s\"\n",
            static_cast<int>(vk_vert_fn.size()), vk_vert_fn.data());
        all_ok = false;
    }

    if (vk_frag_fn != "triangle.frag.spv") {
        std::fprintf(stderr,
            "FAIL: csp_vk_sources[1].filename expected \"triangle.frag.spv\", got \"%.*s\"\n",
            static_cast<int>(vk_frag_fn.size()), vk_frag_fn.data());
        all_ok = false;
    }

    if (triangle_ShaderInfo::csp_vk_sources[0].stage != csp::ShaderStage::Vertex) {
        std::fprintf(stderr, "FAIL: csp_vk_sources[0].stage is not ShaderStage::Vertex\n");
        all_ok = false;
    }

    if (triangle_ShaderInfo::csp_vk_sources[1].stage != csp::ShaderStage::Fragment) {
        std::fprintf(stderr, "FAIL: csp_vk_sources[1].stage is not ShaderStage::Fragment\n");
        all_ok = false;
    }

    // Verify the descriptor points to the same tables and has the right counts.
    if (triangle_descriptor.push_constants      != triangle_ShaderInfo::csp_push_constant_info ||
        triangle_descriptor.push_constant_count != 1u                                          ||
        triangle_descriptor.ogl_sources         != triangle_ShaderInfo::csp_ogl_sources        ||
        triangle_descriptor.vk_sources          != triangle_ShaderInfo::csp_vk_sources         ||
        triangle_descriptor.source_count        != 2u)
    {
        std::fprintf(stderr, "FAIL: triangle_descriptor fields are incorrect\n");
        all_ok = false;
    }

    // Verify get() returns a valid singleton reference.
    const triangle_ShaderInfo& info = triangle_ShaderInfo::get();
    (void)info;

    if (all_ok) {
        std::printf("All checks passed\n");
        return 0;
    } else {
        return 1;
    }
}
