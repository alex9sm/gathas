#include "shadermanager.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

ShaderManager::ShaderManager(VkDevice device) : device(device) {
}

ShaderManager::~ShaderManager() {
    cleanup();
}

VkShaderModule ShaderManager::getShaderModule(const std::string& filename) {
    auto it = shaderCache.find(filename);
    if (it != shaderCache.end()) {
        return it->second;
    }

    std::string fullPath = "shaders/" + filename;
    std::vector<char> shaderCode = readFile(fullPath);

    VkShaderModule shaderModule = createShaderModule(shaderCode);

    shaderCache[filename] = shaderModule;

    std::cout << "Loaded and cached shader: " << filename << std::endl;

    return shaderModule;
}

void ShaderManager::cleanup() {
    for (auto& pair : shaderCache) {
        vkDestroyShaderModule(device, pair.second, nullptr);
    }
    shaderCache.clear();
    std::cout << "cleaned up all cached shaders" << std::endl;
}

std::vector<char> ShaderManager::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule ShaderManager::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }

    return shaderModule;
}
