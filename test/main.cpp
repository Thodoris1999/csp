#include <cstdio>
#include <cstdlib>
#include <filesystem>
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

    // OGL source paths are absolute, verify the files exist.
    constexpr std::string_view ogl_vert = triangle_ShaderInfo::csp_ogl_sources[0].filename;
    constexpr std::string_view ogl_frag = triangle_ShaderInfo::csp_ogl_sources[1].filename;

    if (!std::filesystem::exists(ogl_vert)) {
        std::fprintf(stderr,
            "FAIL: csp_ogl_sources[0].filename does not exist: \"%.*s\"\n",
            static_cast<int>(ogl_vert.size()), ogl_vert.data());
        all_ok = false;
    }

    if (!std::filesystem::exists(ogl_frag)) {
        std::fprintf(stderr,
            "FAIL: csp_ogl_sources[1].filename does not exist: \"%.*s\"\n",
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

    // Vulkan source paths are absolute, verify the files exist.
    constexpr std::string_view vk_vert_fn = triangle_ShaderInfo::csp_vk_sources[0].filename;
    constexpr std::string_view vk_frag_fn = triangle_ShaderInfo::csp_vk_sources[1].filename;

    if (!std::filesystem::exists(vk_vert_fn)) {
        std::fprintf(stderr,
            "FAIL: csp_vk_sources[0].filename does not exist: \"%.*s\"\n",
            static_cast<int>(vk_vert_fn.size()), vk_vert_fn.data());
        all_ok = false;
    }

    if (!std::filesystem::exists(vk_frag_fn)) {
        std::fprintf(stderr,
            "FAIL: csp_vk_sources[1].filename does not exist: \"%.*s\"\n",
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

    // ---------------------------------------------------------------------------
    // Uniform variable reflection
    // ---------------------------------------------------------------------------

    // Fragment shader has two descriptor bindings:
    //   binding 0 – sampler2D tex  (CombinedImageSampler)
    //   binding 1 – uniform MaterialData material (UniformBuffer)
    // Both are fragment-only: VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010.

    constexpr uint32_t frag_stage_flag = 0x00000010u; // VK_SHADER_STAGE_FRAGMENT_BIT

    // --- binding 0: tex (sampler2D → CombinedImageSampler) ---
    constexpr std::string_view tex_name =
        triangle_ShaderInfo::csp_uniform_var_info[0].name;
    if (tex_name != "tex") {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[0].name expected \"tex\", got \"%.*s\"\n",
            static_cast<int>(tex_name.size()), tex_name.data());
        all_ok = false;
    }

    constexpr uint32_t tex_set = triangle_ShaderInfo::csp_uniform_var_info[0].set;
    if (tex_set != 0u) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[0].set expected 0, got %u\n", tex_set);
        all_ok = false;
    }

    constexpr uint32_t tex_binding = triangle_ShaderInfo::csp_uniform_var_info[0].binding;
    if (tex_binding != 0u) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[0].binding expected 0, got %u\n", tex_binding);
        all_ok = false;
    }

    constexpr uint32_t tex_flags = triangle_ShaderInfo::csp_uniform_var_info[0].stage_flags;
    if (tex_flags != frag_stage_flag) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[0].stage_flags expected 0x%08X (fragment), got 0x%08X\n",
            frag_stage_flag, tex_flags);
        all_ok = false;
    }

    constexpr csp::DescriptorType tex_type =
        triangle_ShaderInfo::csp_uniform_var_info[0].descriptor_type;
    if (tex_type != csp::DescriptorType::CombinedImageSampler) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[0].descriptor_type expected CombinedImageSampler\n");
        all_ok = false;
    }

    // --- binding 1: material (UniformBuffer) ---
    constexpr std::string_view mat_name =
        triangle_ShaderInfo::csp_uniform_var_info[1].name;
    if (mat_name != "material") {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[1].name expected \"material\", got \"%.*s\"\n",
            static_cast<int>(mat_name.size()), mat_name.data());
        all_ok = false;
    }

    constexpr uint32_t mat_set = triangle_ShaderInfo::csp_uniform_var_info[1].set;
    if (mat_set != 0u) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[1].set expected 0, got %u\n", mat_set);
        all_ok = false;
    }

    constexpr uint32_t mat_binding = triangle_ShaderInfo::csp_uniform_var_info[1].binding;
    if (mat_binding != 1u) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[1].binding expected 1, got %u\n", mat_binding);
        all_ok = false;
    }

    constexpr uint32_t mat_flags = triangle_ShaderInfo::csp_uniform_var_info[1].stage_flags;
    if (mat_flags != frag_stage_flag) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[1].stage_flags expected 0x%08X (fragment), got 0x%08X\n",
            frag_stage_flag, mat_flags);
        all_ok = false;
    }

    constexpr csp::DescriptorType mat_type =
        triangle_ShaderInfo::csp_uniform_var_info[1].descriptor_type;
    if (mat_type != csp::DescriptorType::UniformBuffer) {
        std::fprintf(stderr,
            "FAIL: csp_uniform_var_info[1].descriptor_type expected UniformBuffer\n");
        all_ok = false;
    }

    // ---------------------------------------------------------------------------
    // Push constant ranges
    // ---------------------------------------------------------------------------

    // pc.transform covers the whole push constant block (offset 0, size 64) and
    // is vertex-only, so there must be exactly one range.
    constexpr uint32_t range_offset = triangle_ShaderInfo::csp_push_constant_ranges[0].offset;
    if (range_offset != 0u) {
        std::fprintf(stderr,
            "FAIL: csp_push_constant_ranges[0].offset expected 0, got %u\n", range_offset);
        all_ok = false;
    }

    constexpr uint32_t range_size = triangle_ShaderInfo::csp_push_constant_ranges[0].size;
    if (range_size != 64u) {
        std::fprintf(stderr,
            "FAIL: csp_push_constant_ranges[0].size expected 64, got %u\n", range_size);
        all_ok = false;
    }

    constexpr uint32_t range_flags = triangle_ShaderInfo::csp_push_constant_ranges[0].stage_flags;
    if (range_flags != 0x00000001u) {
        std::fprintf(stderr,
            "FAIL: csp_push_constant_ranges[0].stage_flags expected 0x00000001 (VK_SHADER_STAGE_VERTEX_BIT), got 0x%08X\n",
            range_flags);
        all_ok = false;
    }

    // ---------------------------------------------------------------------------
    // ProgramDescriptor integrity
    // ---------------------------------------------------------------------------

    // Verify the descriptor points to the same tables and has the right counts.
    if (triangle_descriptor.push_constants           != triangle_ShaderInfo::csp_push_constant_info   ||
        triangle_descriptor.push_constant_count      != 1u                                            ||
        triangle_descriptor.push_constant_ranges     != triangle_ShaderInfo::csp_push_constant_ranges ||
        triangle_descriptor.push_constant_range_count != 1u                                           ||
        triangle_descriptor.uniform_vars             != triangle_ShaderInfo::csp_uniform_var_info     ||
        triangle_descriptor.uniform_count            != 2u                                            ||
        triangle_descriptor.ogl_sources              != triangle_ShaderInfo::csp_ogl_sources          ||
        triangle_descriptor.vk_sources               != triangle_ShaderInfo::csp_vk_sources           ||
        triangle_descriptor.source_count             != 2u)
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
