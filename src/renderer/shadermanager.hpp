#pragma once

#include <Vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <vector>

class ShaderManager {
public:
    ShaderManager(VkDevice device);
    ~ShaderManager();

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    VkShaderModule getShaderModule(const std::string& filename);

    void cleanup();

private:
    VkDevice device;
    std::unordered_map<std::string, VkShaderModule> shaderCache;

    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
};
