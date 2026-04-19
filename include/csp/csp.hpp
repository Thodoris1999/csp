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

struct PushConstantEntry {
    std::string_view name;        // "block.member", e.g. "pc.model"
    uint32_t         offset;
    uint32_t         size;
    uint32_t         stage_flags; // bitmask of VkShaderStageFlagBits
};

struct ShaderSource {
    ShaderStage      stage;
    std::string_view filename;
};

struct ProgramDescriptor {
    const PushConstantEntry* push_constants;
    uint32_t                 push_constant_count;
    const ShaderSource*      ogl_sources;
    const ShaderSource*      vk_sources;
    uint32_t                 source_count;
};

} // namespace csp
