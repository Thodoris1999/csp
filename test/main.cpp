#include <cstdio>
#include <cstdlib>
#include <csp/csp.hpp>             // csp::VkPushConstantEntry, csp::OglUniformEntry
#include "triangle_shader_info.hpp"

int main() {
    bool all_ok = true;

    // Push constant block "pc" is at index 0 in both tables.
    // CSP_UNIFORM_TRIANGLE_PC == 0
    if (CSP_UNIFORM_TRIANGLE_PC != 0) {
        std::fprintf(stderr,
            "FAIL: CSP_UNIFORM_TRIANGLE_PC expected 0, got %d\n",
            CSP_UNIFORM_TRIANGLE_PC);
        all_ok = false;
    }

    // The push constant block covers a mat4 (64 bytes).
    constexpr uint32_t expected_size = 64u;
    constexpr uint32_t actual_size =
        triangle_ShaderInfo::csp_vk_push_constant_info[CSP_UNIFORM_TRIANGLE_PC].size;

    if (actual_size != expected_size) {
        std::fprintf(stderr,
            "FAIL: csp_vk_push_constant_info[PC].size expected %u, got %u\n",
            expected_size, actual_size);
        all_ok = false;
    }

    // Push constant is in the vertex stage.
    constexpr VkShaderStageFlags pc_flags =
        triangle_ShaderInfo::csp_vk_push_constant_info[CSP_UNIFORM_TRIANGLE_PC].stage_flags;

    if ((pc_flags & VK_SHADER_STAGE_VERTEX_BIT) == 0) {
        std::fprintf(stderr,
            "FAIL: push constant stage_flags (0x%08X) does not include VK_SHADER_STAGE_VERTEX_BIT\n",
            pc_flags);
        all_ok = false;
    }

    // The OGL name for the push constant block is "pc" (for glGetUniformBlockIndex).
    constexpr std::string_view pc_ogl_name =
        triangle_ShaderInfo::csp_ogl_uniform_info[CSP_UNIFORM_TRIANGLE_PC].name;

    if (pc_ogl_name != "pc") {
        std::fprintf(stderr,
            "FAIL: csp_ogl_uniform_info[PC].name expected \"pc\", got \"%.*s\"\n",
            static_cast<int>(pc_ogl_name.size()), pc_ogl_name.data());
        all_ok = false;
    }

    // Sampler "tex" follows push constants — index 1.
    // CSP_UNIFORM_TRIANGLE_TEX == 1
    if (CSP_UNIFORM_TRIANGLE_TEX != 1) {
        std::fprintf(stderr,
            "FAIL: CSP_UNIFORM_TRIANGLE_TEX expected 1, got %d\n",
            CSP_UNIFORM_TRIANGLE_TEX);
        all_ok = false;
    }

    constexpr std::string_view tex_name =
        triangle_ShaderInfo::csp_ogl_uniform_info[CSP_UNIFORM_TRIANGLE_TEX].name;

    if (tex_name != "tex") {
        std::fprintf(stderr,
            "FAIL: csp_ogl_uniform_info[TEX].name expected \"tex\", got \"%.*s\"\n",
            static_cast<int>(tex_name.size()), tex_name.data());
        all_ok = false;
    }

    // OGL source filenames (vert then frag, canonical pipeline order).
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

    // Vulkan source filenames and stage flags.
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

    if (triangle_ShaderInfo::csp_vk_sources[0].stage != VK_SHADER_STAGE_VERTEX_BIT) {
        std::fprintf(stderr, "FAIL: csp_vk_sources[0].stage is not VK_SHADER_STAGE_VERTEX_BIT\n");
        all_ok = false;
    }

    if (triangle_ShaderInfo::csp_vk_sources[1].stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
        std::fprintf(stderr, "FAIL: csp_vk_sources[1].stage is not VK_SHADER_STAGE_FRAGMENT_BIT\n");
        all_ok = false;
    }

    // Verify the descriptor points to the same tables and has the right counts.
    if (triangle_descriptor.vk_push_constants != triangle_ShaderInfo::csp_vk_push_constant_info ||
        triangle_descriptor.ogl_uniforms      != triangle_ShaderInfo::csp_ogl_uniform_info      ||
        triangle_descriptor.uniform_count     != 2u                                             ||
        triangle_descriptor.ogl_sources       != triangle_ShaderInfo::csp_ogl_sources           ||
        triangle_descriptor.vk_sources        != triangle_ShaderInfo::csp_vk_sources            ||
        triangle_descriptor.source_count      != 2u)
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
