#include "commandbuffer.hpp"
#include "../ui/imguilayer.hpp"
#include "../core/scene.hpp"
#include "materialmanager.hpp"
#include "indirectdrawing.hpp"
#include <stdexcept>
#include <iostream>

CommandBuffer::CommandBuffer(VkDevice device, uint32_t graphicsQueueFamily, VkQueue graphicsQueue)
    : device(device), commandPool(VK_NULL_HANDLE), graphicsQueue(graphicsQueue) {

    createCommandPool(graphicsQueueFamily);
    createCommandBuffers();
    createSyncObjects();

    std::cout << "command buffer created" << std::endl;
}

//destructor
CommandBuffer::~CommandBuffer() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);
}

void CommandBuffer::createCommandPool(uint32_t graphicsQueueFamily) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsQueueFamily;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

}

void CommandBuffer::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}

void CommandBuffer::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects!");
        }
    }

}

void CommandBuffer::recordGeometryPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
    VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
    VkExtent2D extent, VkPipeline pipeline, VkPipelineLayout pipelineLayout,
    VkDescriptorSet descriptorSet, MaterialManager* materialManager, Scene* scene) {

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 3> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[2].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(extent.height);
    viewport.width = static_cast<float>(extent.width);
    viewport.height = -static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    if (scene && scene->hasUnifiedBuffers()) {
        // bind unified vertex/index buffers once for all draws
        scene->bindUnifiedBuffers(commandBuffer);

        const auto& materialBatches = scene->getMaterialBatches();

        for (const auto& [material, batch] : materialBatches) {
            if (material && material->descriptorSet != VK_NULL_HANDLE) {
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 1, 1, &material->descriptorSet, 0, nullptr);

                // push constants
                struct MaterialPushConstants {
                    glm::vec4 diffuseColor;
                    uint32_t hasTexture;
                    uint32_t hasNormalMap;
                    float dissolve;
                    float padding;
                } pushConstants;

                pushConstants.diffuseColor = material->diffuseColor;
                pushConstants.hasTexture = material->hasTexture ? 1u : 0u;
                pushConstants.hasNormalMap = material->hasNormalMap ? 1u : 0u;
                pushConstants.dissolve = material->dissolve;
                pushConstants.padding = 0.0f;

                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT,
                    0, sizeof(MaterialPushConstants), &pushConstants);
            }

            // issue single indirect draw for all commands in this batch
            if (!batch.drawCommands.empty()) {
                vkCmdDrawIndexedIndirect(
                    commandBuffer,
                    batch.indirectBuffer.getBuffer(),
                    0,
                    static_cast<uint32_t>(batch.drawCommands.size()),
                    sizeof(VkDrawIndexedIndirectCommand)
                );
            }
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}

void CommandBuffer::recordLightingPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
    VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
    VkExtent2D extent, VkPipeline pipeline, VkPipelineLayout pipelineLayout,
    VkDescriptorSet cameraDescriptorSet, VkDescriptorSet gbufferDescriptorSet,
    VkDescriptorSet lightDescriptorSet) {

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    VkClearValue clearValue{};
    clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &cameraDescriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 1, 1, &gbufferDescriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 2, 1, &lightDescriptorSet, 0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(extent.height);
    viewport.width = static_cast<float>(extent.width);
    viewport.height = -static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void CommandBuffer::recordForwardPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
    VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
    VkExtent2D extent, VkPipeline pipeline, VkPipelineLayout pipelineLayout,
    VkDescriptorSet cameraDescriptorSet, VkDescriptorSet lightDescriptorSet,
    Scene* scene, const glm::mat4& viewProj) {

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // only if we have transparent objects
    if (scene && scene->hasTransparentObjects() && scene->hasUnifiedBuffers()) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // bind camera descriptor set
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &cameraDescriptorSet, 0, nullptr);

        // bind light descriptor set
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 2, 1, &lightDescriptorSet, 0, nullptr);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(extent.height);
        viewport.width = static_cast<float>(extent.width);
        viewport.height = -static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // bind unified vertex/index buffers
        scene->bindUnifiedBuffers(commandBuffer);

        // get sorted transparent batches
        auto sortedBatches = scene->getSortedTransparentBatches(viewProj);

        for (const auto& [material, batch] : sortedBatches) {
            if (material && material->descriptorSet != VK_NULL_HANDLE) {
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 1, 1, &material->descriptorSet, 0, nullptr);

                struct MaterialPushConstants {
                    glm::vec4 diffuseColor;
                    uint32_t hasTexture;
                    uint32_t hasNormalMap;
                    float dissolve;
                    float padding;
                } pushConstants;

                pushConstants.diffuseColor = material->diffuseColor;
                pushConstants.hasTexture = material->hasTexture ? 1u : 0u;
                pushConstants.hasNormalMap = material->hasNormalMap ? 1u : 0u;
                pushConstants.dissolve = material->dissolve;
                pushConstants.padding = 0.0f;

                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT,
                    0, sizeof(MaterialPushConstants), &pushConstants);

                // indirect draw for all commands in this batch
                if (!batch->drawCommands.empty()) {
                    vkCmdDrawIndexedIndirect(
                        commandBuffer,
                        batch->indirectBuffer.getBuffer(),
                        0,
                        static_cast<uint32_t>(batch->drawCommands.size()),
                        sizeof(VkDrawIndexedIndirectCommand)
                    );
                }
            }
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}

void CommandBuffer::recordImGuiPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
    VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
    VkExtent2D extent, ImGuiLayer* imguiLayer) {

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    if (imguiLayer) {
        imguiLayer->render(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void CommandBuffer::recordFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkExtent2D extent,
    VkRenderPass geometryRenderPass, const std::vector<VkFramebuffer>& geometryFramebuffers,
    VkPipeline geometryPipeline, VkPipelineLayout geometryPipelineLayout,
    VkDescriptorSet cameraDescriptorSet, MaterialManager* materialManager, Scene* scene,
    VkRenderPass lightingRenderPass, const std::vector<VkFramebuffer>& lightingFramebuffers,
    VkPipeline lightingPipeline, VkPipelineLayout lightingPipelineLayout,
    VkDescriptorSet gbufferDescriptorSet, VkDescriptorSet lightDescriptorSet,
    VkRenderPass forwardRenderPass, const std::vector<VkFramebuffer>& forwardFramebuffers,
    VkPipeline forwardPipeline, VkPipelineLayout forwardPipelineLayout,
    const glm::mat4& viewProj,
    VkRenderPass imguiRenderPass, const std::vector<VkFramebuffer>& imguiFramebuffers,
    ImGuiLayer* imguiLayer) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    recordGeometryPass(commandBuffer, imageIndex, geometryRenderPass, geometryFramebuffers,
        extent, geometryPipeline, geometryPipelineLayout, cameraDescriptorSet,
        materialManager, scene);

    recordLightingPass(commandBuffer, imageIndex, lightingRenderPass, lightingFramebuffers,
        extent, lightingPipeline, lightingPipelineLayout, cameraDescriptorSet,
        gbufferDescriptorSet, lightDescriptorSet);

    recordForwardPass(commandBuffer, imageIndex, forwardRenderPass, forwardFramebuffers,
        extent, forwardPipeline, forwardPipelineLayout, cameraDescriptorSet,
        lightDescriptorSet, scene, viewProj);

    recordImGuiPass(commandBuffer, imageIndex, imguiRenderPass, imguiFramebuffers,
        extent, imguiLayer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

VkCommandBuffer CommandBuffer::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CommandBuffer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}