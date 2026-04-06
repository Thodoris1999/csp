#pragma once
#include <cstdint>
#include <string_view>
#include <vulkan/vulkan.h>

namespace csp {

struct VkPushConstantEntry {
    uint32_t           offset;
    uint32_t           size;
    VkShaderStageFlags stage_flags;
};

struct OglUniformEntry {
    std::string_view name;
};

struct OglShaderSource {
    std::string_view filename;
};

struct VkShaderSource {
    VkShaderStageFlagBits stage;
    std::string_view   filename;
};

// Stable descriptor passed to a graphics backend. Both uniform arrays share
// the same index space, so a single CSP_UNIFORM_* macro indexes into both.
struct ProgramDescriptor {
    const VkPushConstantEntry* vk_push_constants;
    const OglUniformEntry*     ogl_uniforms;
    uint32_t                   uniform_count;
    const OglShaderSource*     ogl_sources;
    const VkShaderSource*      vk_sources;
    uint32_t                   source_count;
};

} // namespace csp
