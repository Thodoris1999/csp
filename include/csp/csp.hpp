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

enum class DescriptorType {
    Sampler,                 // VK_DESCRIPTOR_TYPE_SAMPLER
    CombinedImageSampler,    // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    SampledImage,            // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    StorageImage,            // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    UniformTexelBuffer,      // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    StorageTexelBuffer,      // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    UniformBuffer,           // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    StorageBuffer,           // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    UniformBufferDynamic,    // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    StorageBufferDynamic,    // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    InputAttachment,         // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    AccelerationStructure,   // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
};

struct PushConstantEntry {
    std::string_view name;        // "block.member", e.g. "pc.model"
    uint32_t         offset;
    uint32_t         size;
    uint32_t         stage_flags; // bitmask of VkShaderStageFlagBits
};

struct UniformVar {
    std::string_view name;
    uint32_t         set;
    uint32_t         binding;
    uint32_t         stage_flags;    // bitmask of VkShaderStageFlagBits
    DescriptorType   descriptor_type;
};

struct ShaderSource {
    ShaderStage      stage;
    std::string_view filename;
};

struct ProgramDescriptor {
    const PushConstantEntry* push_constants;
    uint32_t                 push_constant_count;
    const UniformVar*        uniform_vars;
    uint32_t                 uniform_count;
    const ShaderSource*      ogl_sources;
    const ShaderSource*      vk_sources;
    uint32_t                 source_count;
};

} // namespace csp
