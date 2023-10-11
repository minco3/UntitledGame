#pragma once

#include "Buffer.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "Shader.hpp"
#include "Surface.hpp"
#include "UniformBuffer.hpp"
#include "Vertex.hpp"
#include "Window.hpp"
#include "Swapchain.hpp"
#include <vector>
#include <vulkan/vulkan.h>

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

    Window m_Window;
    VulkanInstance m_Instance;
    Surface m_Surface;
    Device m_Device;
    Swapchain m_Swapchain;
    std::vector<VkFramebuffer> m_Framebuffers;
    std::vector<Shader> m_Shaders;
    VkRenderPass m_RenderPass;
    VkCommandPool m_CommandPool;
    VkCommandBuffer m_CommandBuffer;
    Buffer<Vertex> m_VertexBuffer;
    std::vector<Buffer<UniformBufferObject>> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBufferMemory;
    std::vector<void*> m_UniformBufferMemoryMapped;
    VkDeviceMemory m_VertexBufferMemory;
    uint32_t m_QueueFamilyIndex = 0;
    VkQueue m_Queue;
    VkPipeline m_Pipeline;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;
    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkPipelineLayout m_PipelineLayout;
    VkSemaphore m_Semaphore;
    VkSemaphore m_RenderSemaphore;
    VkFence m_Fence;
    uint32_t m_ImageIndex = 0;
};