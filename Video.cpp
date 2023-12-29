#include "Video.hpp"
#include "Buffer.hpp"
#include "Log.hpp"
#include "UniformBuffer.hpp"
#include "Vertex.hpp"
#include "Pipeline.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <fmt/format.h>
#include <glm/common.hpp>
#include <glm/mat2x2.hpp>
#include <span>
#include <vulkan/vulkan_beta.h>

Video::Video()
    : m_Window(
          "Untitled Game", {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED},
          {1200, 800}, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN),
      m_Instance(m_Window, m_Context), m_Surface(m_Window, m_Instance),
      m_Device(m_Instance, m_Surface), m_Swapchain(m_Device, m_Surface),
      m_Pipeline(),

      
{

    //vertebuffer
    CreateVertexBuffer();
    
    // framebuffer
    CreateFramebuffers();
    
    // buffers
    CreateCommandBuffer();
    CreateUniformBuffers();

    //descriptor set
    CreateDescriptorPool();
    CreateDescriptorSets();

    CreateMemoryBarriers();
}
Video::~Video()
{
    VkResult result;
    result = vkQueueWaitIdle(m_Queue);
    LogVulkanError("Queue Wait Error", result);
    result = vkDeviceWaitIdle(m_Device());
    LogVulkanError("Device Wait Error", result);
    vkDestroyFence(m_Device(), m_Fence, nullptr);
    vkDestroySemaphore(m_Device(), m_Semaphore, nullptr);
    vkDestroySemaphore(m_Device(), m_RenderSemaphore, nullptr);
    vkDestroyBuffer(m_Device(), m_VertexBuffer(), nullptr);
    vkFreeMemory(m_Device(), m_VertexBufferMemory, nullptr);
    for (auto uniformBuffer : m_UniformBuffers)
    {
        vkDestroyBuffer(m_Device(), uniformBuffer(), nullptr);
    }
    for (VkDeviceMemory uniformBufferMemory : m_UniformBufferMemory)
    {
        vkFreeMemory(m_Device(), uniformBufferMemory, nullptr);
    }
    for (VkImageView imageView : m_Swapchain.m_SwapchainImageViews)
    {
        vkDestroyImageView(m_Device(), imageView, nullptr);
    }
    vkDestroyPipelineLayout(m_Device(), m_PipelineLayout, nullptr);
    vkDestroyDescriptorPool(m_Device(), m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device(), m_DescriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_Device(), m_RenderPass, nullptr);
    for (VkFramebuffer framebuffer : m_Framebuffers)
    {
        vkDestroyFramebuffer(m_Device(), framebuffer, nullptr);
    }
    vkDestroyCommandPool(m_Device(), m_CommandPool, nullptr);
    for (Shader& shader : m_Shaders)
    {
        shader.DestroyShaderModules();
    }
    vkDestroySwapchainKHR(m_Device(), m_Swapchain(), nullptr);
    vkDestroyPipeline(m_Device(), m_Pipeline, nullptr);
    vkDestroyDevice(m_Device(), nullptr);
    vkDestroySurfaceKHR(m_Instance(), m_Surface(), nullptr);
    vkDestroyInstance(m_Instance(), nullptr);
}

void Video::Render()
{
    VkResult result;
    vkWaitForFences(
        m_Device(), 1, &m_Fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(m_Device(), 1, &m_Fence);
    result = vkAcquireNextImageKHR(
        m_Device(), m_Swapchain(), std::numeric_limits<uint64_t>::max(),
        m_Semaphore, VK_NULL_HANDLE, &m_ImageIndex);
    LogVulkanError("failed to aquire next image", result);

    vkResetCommandBuffer(m_CommandBuffer, 0);

    VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkClearValue clearValue = {.color = clearColor};

    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_RenderPass,
        .framebuffer = m_Framebuffers.at(m_ImageIndex),
        .renderArea =
            {.offset = {0, 0}, .extent = m_Surface.surfaceCapabilities.currentExtent},
        .clearValueCount = 1,
        .pClearValues = &clearValue};

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT};

    result = vkBeginCommandBuffer(m_CommandBuffer, &commandBufferBeginInfo);
    LogVulkanError("failed to start recording command buffer", result);

    vkCmdBeginRenderPass(
        m_CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &m_VertexBuffer(), &offset);

    vkCmdBindDescriptorSets(
        m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0,
        1, &m_DescriptorSets.at(m_ImageIndex), 0, nullptr);

    vkCmdDraw(m_CommandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    vkCmdEndRenderPass(m_CommandBuffer);

    result = vkEndCommandBuffer(m_CommandBuffer);
    LogVulkanError("failed to stop recording command buffer", result);

    m_Queue = GetQueue(m_QueueFamilyIndex, 0);

    VkPipelineStageFlags waitFlags =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_Semaphore,
        .pWaitDstStageMask = &waitFlags,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_CommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_RenderSemaphore};
    result = vkQueueSubmit(m_Queue, 1, &submitInfo, m_Fence);
    LogVulkanError("failed to submit draw command buffer", result);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_RenderSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain(),
        .pImageIndices = &m_ImageIndex};
    result = vkQueuePresentKHR(m_Queue, &presentInfo);
    LogVulkanError("failed to present queue", result);
}

void Video::UpdateUnformBuffers(float theta)
{
    UniformBufferObject* buffer = static_cast<UniformBufferObject*>(
        m_UniformBufferMemoryMapped.at(m_ImageIndex));
    UniformBufferObject ubo;
    ubo.rotation[0].x = cos(theta * M_PI / 180);
    ubo.rotation[0].y = -sin(theta * M_PI / 180);
    ubo.rotation[1].x = sin(theta * M_PI / 180);
    ubo.rotation[1].y = cos(theta * M_PI / 180);
    ubo.colorRotation = theta;
    *buffer = ubo;
}

void Video::CreateFramebuffers()
{
    VkResult result;
    assert(m_Swapchain.m_SwapchainImageViews.size() > 0);
    m_Framebuffers.resize(m_Swapchain.m_SwapchainImageViews.size());
    for (size_t i = 0; i < m_Swapchain.m_SwapchainImageViews.size(); i++)
    {
        VkFramebuffer& framebuffer = m_Framebuffers.at(i);
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_RenderPass,
            .attachmentCount = 1,
            .pAttachments = &m_Swapchain.m_SwapchainImageViews.at(i),
            .width = m_Surface.surfaceCapabilities.currentExtent.width,
            .height = m_Surface.surfaceCapabilities.currentExtent.height,
            .layers = 1};

        result = vkCreateFramebuffer(
            m_Device(), &framebufferCreateInfo, nullptr, &framebuffer);
        LogVulkanError(
            fmt::format("failed to create framebuffer {}", i), result);
    }
}

void Video::CreateCommandBuffer()
{
    VkResult result;
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_QueueFamilyIndex};
    result = vkCreateCommandPool(
        m_Device(), &commandPoolCreateInfo, nullptr, &m_CommandPool);
    LogVulkanError("failed to create command pool", result);

    VkCommandBufferAllocateInfo commandBuffersAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};
    result = vkAllocateCommandBuffers(
        m_Device(), &commandBuffersAllocateInfo, &m_CommandBuffer);
    LogVulkanError("failed to allocate command buffers", result);
}

void Video::CreateVertexBuffer()
{

    m_VertexBuffer.Create(
        m_Device, vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_VertexBufferMemory);

    // load hard-coded vertices into memory
    void* data;
    vkMapMemory(
        m_Device(), m_VertexBufferMemory, 0, m_VertexBuffer.size(), 0, &data);
    std::span<Vertex> memorySpan(static_cast<Vertex*>(data), vertices.size());
    std::copy_n(vertices.begin(), vertices.size(), memorySpan.begin());
    vkUnmapMemory(m_Device(), m_VertexBufferMemory);
}

void Video::CreateUniformBuffers()
{
    VkResult result;
    m_UniformBuffers.resize(m_Swapchain.m_SwapchainImageViews.size());
    m_UniformBufferMemoryMapped.resize(m_Swapchain.m_SwapchainImageViews.size());
    m_UniformBufferMemory.resize(m_Swapchain.m_SwapchainImageViews.size());
    for (size_t i = 0; i < m_UniformBuffers.size(); i++)
    {
        m_UniformBuffers.at(i).Create(
            m_Device, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_UniformBufferMemory.at(i));

        result = vkMapMemory(
            m_Device(), m_UniformBufferMemory.at(i), 0,
            m_UniformBuffers.at(i).size(), 0,
            &m_UniformBufferMemoryMapped.at(i));
        LogVulkanError("failed to map uniform buffer memory", result);
    }
}

void Video::CreateDescriptorPool()
{
    VkResult result;
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t>(m_Swapchain.m_SwapchainImageViews.size())};

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize};

    poolInfo.maxSets = static_cast<uint32_t>(m_Swapchain.m_SwapchainImageViews.size());
    result = vkCreateDescriptorPool(
        m_Device(), &poolInfo, nullptr, &m_DescriptorPool);
    LogVulkanError("Failed to create descriptor pool", result);
}

void Video::CreateDescriptorSets()
{
    VkResult result;
    std::vector<VkDescriptorSetLayout> layouts(
        m_Swapchain.m_SwapchainImageViews.size(), m_DescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_DescriptorPool,
        .descriptorSetCount =
            static_cast<uint32_t>(m_Swapchain.m_SwapchainImageViews.size()),
        .pSetLayouts = layouts.data()};

    m_DescriptorSets.resize(m_Swapchain.m_SwapchainImageViews.size());
    result = vkAllocateDescriptorSets(
        m_Device(), &allocInfo, m_DescriptorSets.data());

    for (size_t i = 0; i < m_Swapchain.m_SwapchainImageViews.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = m_UniformBuffers.at(i)(),
            .offset = 0,
            .range = sizeof(UniformBufferObject)};

        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_DescriptorSets.at(i),
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo};

        vkUpdateDescriptorSets(m_Device(), 1, &descriptorWrite, 0, nullptr);
    }
}

void Video::CreateMemoryBarriers()
{
    VkResult result;
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    result = vkCreateSemaphore(
        m_Device(), &semaphoreCreateInfo, nullptr, &m_Semaphore);
    LogVulkanError("failed to create semaphore", result);
    result = vkCreateSemaphore(
        m_Device(), &semaphoreCreateInfo, nullptr, &m_RenderSemaphore);
    LogVulkanError("failed to create semaphore", result);
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    result = vkCreateFence(m_Device(), &fenceCreateInfo, nullptr, &m_Fence);
    LogVulkanError("failed to create fence", result);
}

VkQueue Video::GetQueue(uint32_t queueFamily, uint32_t index)
{
    VkQueue queue;
    vkGetDeviceQueue(m_Device(), index, 0, &queue);
    return queue;
}