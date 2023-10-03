#include "Application.hpp"
#include "Log.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_vulkan.h>
#include <array>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fstream>
#include <glm/glm.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>

// #define DEBUG

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        LogError(fmt::format("Error initializing sdl: {}", SDL_GetError()));
    }
    try
    {
        Application app;
        app.Run();
    }
    catch (std::runtime_error& e)
    {
        LogError(fmt::format("Runtime Error: {}\n", e.what()));
        exit(1);
    }
    SDL_Quit();

    VkResult result;
    bool running = true;

    const uint32_t queueFamilyIndex = 0;

    VkQueue queue;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    VkCommandPool commandPool;
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex};
    result = vkCreateCommandPool(
        device, &commandPoolCreateInfo, nullptr, &commandPool);
    LogVulkanError("failed to create command pool", result);

    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo commandBuffersAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};
    result = vkAllocateCommandBuffers(
        device, &commandBuffersAllocateInfo, &commandBuffer);
    LogVulkanError("failed to allocate command buffers", result);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT};

    VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkClearValue clearValue = {.color = clearColor};

    const std::vector<Vertex> vertices = {
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    VkBuffer vertexBuffer;
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vertices.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &vertexBuffer);
    LogVulkanError("failed to create vertex buffer", result);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t memoryTypeIndex;

    LogDebug(
        fmt::format("TypeFilter: {:#b}", memoryRequirements.memoryTypeBits));

    LogDebug(fmt::format("Available Memory Types:"));
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        LogDebug(fmt::format(
            "\t{:#b}", static_cast<uint32_t>(
                           memoryProperties.memoryTypes[i].propertyFlags)));
    }

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
                properties)
        {
            memoryTypeIndex = i;
            break;
        }
        if (i == memoryProperties.memoryTypeCount - 1)
        {
            LogError("ERROR: failed to find suitable memory type\n");
        }
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex};

    VkDeviceMemory vertexBufferMemory;

    result = vkAllocateMemory(
        device, &memoryAllocateInfo, nullptr, &vertexBufferMemory);
    LogVulkanError("failed to allocate vertex buffer memory", result);

    vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferCreateInfo.size, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferCreateInfo.size));
    vkUnmapMemory(device, vertexBufferMemory);

    VkSemaphore semaphore = {};
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    result =
        vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
    LogVulkanError("failed to create semaphore", result);
    VkSemaphore renderSemaphore = {};
    result = vkCreateSemaphore(
        device, &semaphoreCreateInfo, nullptr, &renderSemaphore);
    LogVulkanError("failed to create semaphore", result);
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    result = vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
    LogVulkanError("failed to create fence", result);

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    running = false;
                    break;
                }
            }
        }

        vkWaitForFences(
            device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(device, 1, &fence);
        uint32_t imageIndex = 0;
        result = vkAcquireNextImageKHR(
            device, swapchain, std::numeric_limits<uint64_t>::max(), semaphore,
            VK_NULL_HANDLE, &imageIndex);
        LogVulkanError("failed to aquire next image", result);

        vkResetCommandBuffer(commandBuffer, 0);

        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderPass,
            .framebuffer = framebuffers.at(imageIndex),
            .renderArea =
                {.offset = {0, 0}, .extent = surfaceCapabilities.currentExtent},
            .clearValueCount = 1,
            .pClearValues = &clearValue};

        result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
        LogVulkanError("failed to start recording command buffer", result);

        vkCmdBeginRenderPass(
            commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

        vkCmdDraw(
            commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        LogVulkanError("failed to stop recording command buffer", result);

        VkPipelineStageFlags waitFlags =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &semaphore,
            .pWaitDstStageMask = &waitFlags,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderSemaphore};
        result = vkQueueSubmit(queue, 1, &submitInfo, fence);
        LogVulkanError("failed to submit draw command buffer", result);

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIndex};
        result = vkQueuePresentKHR(queue, &presentInfo);
        LogVulkanError("failed to present queue", result);
    }

    return 0;
}