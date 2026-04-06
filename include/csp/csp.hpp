#pragma once
#include <cstdint>
#include <string_view>

namespace csp {

enum class ShaderStage {
    Vertex,
    TessControl,
    TessEval,
    Geometry,
    Fragment,
    Compute,
};

struct VkPushConstantEntry {
    uint32_t offset;
    uint32_t size;
    uint32_t stage_flags; // bitmask of VkShaderStageFlagBits
};

struct OglUniformEntry {
    std::string_view name;
};

struct ShaderSource {
    ShaderStage      stage;
    std::string_view filename;
};

// Stable descriptor passed to a graphics backend. Both uniform arrays share
// the same index space, so a single CSP_UNIFORM_* macro indexes into both.
struct ProgramDescriptor {
    const VkPushConstantEntry* vk_push_constants;
    const OglUniformEntry*     ogl_uniforms;
    uint32_t                   uniform_count;
    const ShaderSource*        ogl_sources;
    const ShaderSource*        vk_sources;
    uint32_t                   source_count;
};

} // namespace csp
