#pragma once

#include "Buffer.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "Shader.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "UniformBuffer.hpp"
#include "Vertex.hpp"
#include "Window.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class Video
{
public:
    Video();
    ~Video();

    void Render();
    void UpdateUnformBuffers(float theta);

private:
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandBuffer();
    void CreateVertexBuffer();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateMemoryBarriers();
    void CreatePipelineLayout();

    std::vector<VkPipelineShaderStageCreateInfo> CreatePipelineShaderStage();
    VkQueue GetQueue(uint32_t queueFamily, uint32_t index);

    vk::raii::Context m_Context;
    Window m_Window;
    VulkanInstance m_Instance;
    Surface m_Surface;
    Device m_Device;
    Swapchain m_Swapchain;
    std::vector<vk::raii::Framebuffer> m_Framebuffers;
    std::vector<Shader> m_Shaders;
    vk::raii::CommandPool m_CommandPool;
    vk::raii::CommandBuffer m_CommandBuffer;
    vk::raii::RenderPass m_RenderPass;
    Buffer<Vertex> m_VertexBuffer;
    std::vector<Buffer<UniformBufferObject>> m_UniformBuffers;
    std::vector<vk::raii::DeviceMemory> m_UniformBufferMemory;
    std::vector<void*> m_UniformBufferMemoryMapped;
    vk::raii::DeviceMemory m_VertexBufferMemory;
    uint32_t m_QueueFamilyIndex = 0;
    vk::raii::Queue m_Queue;
    GraphicsPipeline m_Pipeline;
    vk::raii::DescriptorPool m_DescriptorPool;
    vk::raii::DescriptorSets m_DescriptorSets;
    vk::raii::DescriptorSetLayout m_DescriptorSetLayout;
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::Semaphore m_Semaphore;
    vk::raii::Semaphore m_RenderSemaphore;
    vk::raii::Fence m_Fence;
    uint32_t m_ImageIndex = 0;
};