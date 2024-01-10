#pragma once

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Descriptors.hpp"
#include "Device.hpp"
#include "Framebuffers.hpp"
#include "Instance.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include "SyncObjects.hpp"
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
    void InitImGui();
    
private:
    void FillVertexBuffer();
    std::vector<Buffer<UniformBufferObject>> ConstructUniformBuffers();

    vk::raii::Context m_Context;
    Window m_Window;
    VulkanInstance m_Instance;
    Surface m_Surface;
    Device m_Device;
    uint32_t m_QueueFamilyIndex = 0;
    vk::raii::Queue m_Queue;
    Swapchain m_Swapchain;
    RenderPass m_RenderPass;
    Framebuffers m_Framebuffers;
    Buffer<Vertex> m_VertexBuffer;
    SyncObjects m_SyncObjects;
    std::vector<Buffer<UniformBufferObject>> m_UniformBuffers;
    uint32_t m_CurrentImage = 0;
    CommandBuffer m_CommandBuffers;
    Descriptors m_Descriptors;
    GraphicsPipeline m_Pipeline;
};