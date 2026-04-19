#include <cstdio>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "spirv_reflect.h"

static VkShaderStageFlagBits stage_flag_from_name(const std::string& stage) {
    if (stage == "vert") return VK_SHADER_STAGE_VERTEX_BIT;
    if (stage == "tesc") return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (stage == "tese") return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (stage == "geom") return VK_SHADER_STAGE_GEOMETRY_BIT;
    if (stage == "frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
    if (stage == "comp") return VK_SHADER_STAGE_COMPUTE_BIT;
    throw std::runtime_error("Unknown shader stage name " + stage);
}

static std::string shader_stage_format(const std::string& stage) {
    if (stage == "vert") return "csp::ShaderStage::Vertex";
    if (stage == "tesc") return "csp::ShaderStage::TessControl";
    if (stage == "tese") return "csp::ShaderStage::TessEval";
    if (stage == "geom") return "csp::ShaderStage::Geometry";
    if (stage == "frag") return "csp::ShaderStage::Fragment";
    if (stage == "comp") return "csp::ShaderStage::Compute";
    throw std::runtime_error("Unknown shader stage name " + stage);
}

static std::string format_stage_flags(uint32_t flags) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "0x%08X", flags);
    return std::string(buf);
}

struct UniformEntry {
    std::string        name;        // "block.member", e.g. "pc.model"
    std::string        define_name; // "block_member", e.g. "pc_model"
    int                index;
    uint32_t           offset;
    uint32_t           size;
    VkShaderStageFlags stage_flags;
};

static std::vector<uint32_t> read_spirv(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open SPIR-V file: " + path);
    }
    std::streamsize byte_size = f.tellg();
    f.seekg(0, std::ios::beg);
    if (byte_size % 4 != 0) {
        throw std::runtime_error("SPIR-V file size not aligned to 4 bytes: " + path);
    }
    std::vector<uint32_t> words(static_cast<size_t>(byte_size) / 4);
    f.read(reinterpret_cast<char*>(words.data()), byte_size);
    return words;
}

struct StageReflection {
    std::vector<UniformEntry> push_constants;
};

static StageReflection reflect_stage(const std::string& spv_path, const std::string& stage) {
    uint32_t stage_flag = stage_flag_from_name(stage);

    std::vector<uint32_t> spirv = read_spirv(spv_path);

    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(
        spirv.size() * sizeof(uint32_t), spirv.data(), &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("spvReflectCreateShaderModule failed for: " + spv_path);
    }

    StageReflection sr;

    // Push constants — enumerate per-member
    uint32_t pc_count = 0;
    result = spvReflectEnumeratePushConstantBlocks(&module, &pc_count, nullptr);
    if (result == SPV_REFLECT_RESULT_SUCCESS && pc_count > 0) {
        std::vector<SpvReflectBlockVariable*> pc_blocks(pc_count);
        spvReflectEnumeratePushConstantBlocks(&module, &pc_count, pc_blocks.data());
        for (auto* blk : pc_blocks) {
            std::string block_name = blk->name ? blk->name : "pc";
            for (uint32_t m = 0; m < blk->member_count; ++m) {
                const SpvReflectBlockVariable& mem = blk->members[m];
                std::string member_name = mem.name ? mem.name : std::to_string(m);
                UniformEntry e;
                e.name        = block_name + "." + member_name;
                e.define_name = block_name + "_" + member_name;
                e.index       = 0; // assigned later
                e.offset      = mem.offset;
                e.size        = mem.size;
                e.stage_flags = stage_flag;
                sr.push_constants.push_back(e);
            }
        }
    }

    spvReflectDestroyShaderModule(&module);
    return sr;
}

static std::vector<UniformEntry> merge_stages(
    const std::vector<std::pair<std::string, std::string>>& stage_spv_pairs)
{
    std::map<std::string, size_t> seen_pc;
    std::vector<UniformEntry>     merged;

    for (auto& [stage, spv_path] : stage_spv_pairs) {
        StageReflection sr = reflect_stage(spv_path, stage);

        for (auto& e : sr.push_constants) {
            auto it = seen_pc.find(e.name);
            if (it == seen_pc.end()) {
                seen_pc[e.name] = merged.size();
                merged.push_back(e);
            } else {
                merged[it->second].stage_flags |= e.stage_flags;
            }
        }
    }

    for (int i = 0; i < static_cast<int>(merged.size()); ++i) {
        merged[i].index = i;
    }

    return merged;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr,
            "Usage: %s <program_name> <output_dir> <templates_dir> <stage1:spv1> [<stage2:spv2> ...]\n",
            argv[0]);
        return 1;
    }

    std::string program_name  = argv[1];
    std::string output_dir    = argv[2];
    std::string templates_dir = argv[3];

    if (argc < 5) {
        std::fprintf(stderr, "Error: No stage:spv arguments provided.\n");
        return 1;
    }

    std::vector<std::pair<std::string, std::string>> stage_spv_pairs;
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        auto colon = arg.find(':');
        if (colon == std::string::npos) {
            std::fprintf(stderr, "Error: Expected 'stage:spv_file' but got '%s'\n", arg.c_str());
            return 1;
        }
        std::string stage    = arg.substr(0, colon);
        std::string spv_path = arg.substr(colon + 1);
        stage_spv_pairs.emplace_back(stage, spv_path);
    }

    std::vector<UniformEntry> uniforms;
    try {
        uniforms = merge_stages(stage_spv_pairs);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "Error during reflection: %s\n", ex.what());
        return 1;
    }

    // Build shader source tables (one entry per stage, in argument order)
    nlohmann::json vk_sources  = nlohmann::json::array();
    nlohmann::json ogl_sources = nlohmann::json::array();
    for (auto& [stage, spv_path] : stage_spv_pairs) {
        size_t slash        = spv_path.rfind('/');
        std::string spv_fn  = (slash == std::string::npos) ? spv_path : spv_path.substr(slash + 1);
        // Derive OGL filename: strip ".spv", append ".ogl.glsl"
        std::string ogl_fn  = spv_fn.substr(0, spv_fn.size() - 4) + ".ogl.glsl";

        nlohmann::json vks;
        vks["stage"]    = shader_stage_format(stage);
        vks["filename"] = spv_fn;
        vk_sources.push_back(vks);

        nlohmann::json ogls;
        ogls["stage"]    = shader_stage_format(stage);
        ogls["filename"] = ogl_fn;
        ogl_sources.push_back(ogls);
    }

    // Build JSON data for inja
    nlohmann::json data;
    data["program_name"]   = program_name;
    data["uniform_count"]  = uniforms.size();
    data["source_count"]   = stage_spv_pairs.size();
    data["vk_sources"]     = vk_sources;
    data["ogl_sources"]    = ogl_sources;
    data["uniforms"]       = nlohmann::json::array();

    for (auto& u : uniforms) {
        nlohmann::json ju;
        ju["name"]        = u.name;
        ju["define_name"] = u.define_name;
        ju["index"]       = u.index;
        ju["offset"]      = u.offset;
        ju["size"]        = u.size;
        ju["stage_flags"] = format_stage_flags(u.stage_flags);
        data["uniforms"].push_back(ju);
    }

    // Render templates
    inja::Environment env(templates_dir + "/");
    env.set_statement("{%", "%}");
    env.set_line_statement("##");

    struct TemplateSpec {
        std::string template_file;
        std::string output_file;
    };

    std::vector<TemplateSpec> specs = {
        { "shader_info.hpp.inja", program_name + "_shader_info.hpp" },
        { "shader_info.cpp.inja", program_name + "_shader_info.cpp" },
    };

    for (auto& spec : specs) {
        std::string out_path = output_dir + "/" + spec.output_file;
        try {
            env.write(spec.template_file, data, out_path);
        } catch (const std::exception& ex) {
            std::fprintf(stderr, "Error rendering template '%s': %s\n",
                spec.template_file.c_str(), ex.what());
            return 1;
        }
        std::printf("csp: Generated %s\n", out_path.c_str());
    }

    return 0;
}
