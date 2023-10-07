#pragma once

#include "DeviceQueue.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include <vector>
#include <vulkan/vulkan.h>

class Video
{
public:
    Video();
    ~Video();

    void Render();
    void UpdateUnformBuffers();

private:
    void CreateInstance();
    void CreateDevice();
    void CreateSwapchain();
    void CreateSwapchainImageViews();
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
    std::vector<const char*> GetExtensionNames();
    std::vector<VkExtensionProperties> GetInstanceSupportedExtensions();
    std::vector<VkPhysicalDevice> GetPhysicalDevices();
    std::vector<VkDeviceQueueCreateInfo>
    GetDeviceQueueCreateInfos(VkPhysicalDevice device);
    std::vector<const char*> GetDeviceExtentionNames(VkPhysicalDevice device);
    bool CheckDeviceSupportsPresentation(VkPhysicalDevice device);
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities();
    VkSurfaceFormatKHR GetSurfaceFormat();
    const std::vector<VkSurfaceFormatKHR> GetCompatableSurfaceFormats();
    VkSurfaceFormatKHR PickSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& surfaceFormats,
        const VkFormat requestedFormat);
    std::vector<VkImage> GetSwapchainImages();
    VkQueue GetQueue(uint32_t queueFamily, uint32_t index);
    VkInstanceCreateFlags
    GetInstanceCreateFlags(const std::vector<const char*>& extentionNames);
    uint32_t FindMemoryType(
        VkMemoryRequirements VkMemoryRequirements,
        VkMemoryPropertyFlags memoryPropertyFlags);
    void CreateBuffer(
        VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags propertyFlags, VkDeviceMemory& bufferMemory,
        VkBuffer& buffer);

    std::vector<DeviceQueue> m_DeviceQueues;
    VkInstance m_Instance;
    Window m_Window;
    VkSurfaceKHR m_Surface;
    VkSurfaceFormatKHR m_SurfaceFormat;
    VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSwapchainKHR m_Swapchain;
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;
    std::vector<Shader> m_Shaders;
    VkRenderPass m_RenderPass;
    VkCommandPool m_CommandPool;
    VkCommandBuffer m_CommandBuffer;
    VkBuffer m_VertexBuffer;
    std::vector<VkBuffer> m_UniformBuffers;
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