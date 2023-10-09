#include "Video.hpp"
#include "Log.hpp"
#include "Vertex.hpp"
#include "UniformBuffer.hpp"
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
          {1200, 800}, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN)
{
    CreateInstance();
    m_Surface = m_Window.CreateSDLSurface(m_Instance);
    CreateDevice();
    m_SurfaceCapabilities = GetSurfaceCapabilities();
    CreateSwapchain();
    CreateSwapchainImageViews();
    CreatePipelineLayout();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffer();
    CreateVertexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateMemoryBarriers();
}
Video::~Video()
{
    VkResult result;
    result = vkQueueWaitIdle(m_Queue);
    LogVulkanError("Queue Wait Error", result);
    result = vkDeviceWaitIdle(m_Device);
    LogVulkanError("Device Wait Error", result);
    vkDestroyFence(m_Device, m_Fence, nullptr);
    vkDestroySemaphore(m_Device, m_Semaphore, nullptr);
    vkDestroySemaphore(m_Device, m_RenderSemaphore, nullptr);
    vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
    for (VkBuffer uniformBuffer : m_UniformBuffers)
    {
        vkDestroyBuffer(m_Device, uniformBuffer, nullptr);
    }
    for (VkDeviceMemory uniformBufferMemory : m_UniformBufferMemory)
    {
        vkFreeMemory(m_Device, uniformBufferMemory, nullptr);
    }
    for (VkImageView imageView : m_SwapchainImageViews)
    {
        vkDestroyImageView(m_Device, imageView, nullptr);
    }
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    for (VkFramebuffer framebuffer : m_Framebuffers)
    {
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    }
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    for (Shader& shader : m_Shaders)
    {
        shader.DestroyShaderModules();
    }
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void Video::Render()
{
    VkResult result;
    vkWaitForFences(
        m_Device, 1, &m_Fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(m_Device, 1, &m_Fence);
    result = vkAcquireNextImageKHR(
        m_Device, m_Swapchain, std::numeric_limits<uint64_t>::max(),
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
            {.offset = {0, 0}, .extent = m_SurfaceCapabilities.currentExtent},
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
    vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &m_VertexBuffer, &offset);

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
        .pSwapchains = &m_Swapchain,
        .pImageIndices = &m_ImageIndex};
    result = vkQueuePresentKHR(m_Queue, &presentInfo);
    LogVulkanError("failed to present queue", result);
}

void Video::UpdateUnformBuffers(float theta)
{
    UniformBufferObject* buffer =
        static_cast<UniformBufferObject*>(m_UniformBufferMemoryMapped.at(m_ImageIndex));
    UniformBufferObject ubo;
    ubo.rotation[0].x = cos(theta*M_PI/180);
    ubo.rotation[0].y = -sin(theta*M_PI/180);
    ubo.rotation[1].x = sin(theta*M_PI/180);
    ubo.rotation[1].y = cos(theta*M_PI/180);
    *buffer = ubo;
}

void Video::CreateInstance()
{
    VkResult result;

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Untitled Game",
        .applicationVersion = 1,
        .pEngineName = nullptr,
        .apiVersion = VK_API_VERSION_1_3};

    std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char*> extNames = GetExtensionNames();

    VkInstanceCreateFlags instanceCreateFlags =
        GetInstanceCreateFlags(extNames);

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = instanceCreateFlags,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extNames.size()),
        .ppEnabledExtensionNames = extNames.data()};

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
    LogVulkanError("Could not create vulkan instance", result);
}

void Video::CreateDevice()
{
    VkResult result;
    std::vector<VkPhysicalDevice> physicalDevices = GetPhysicalDevices();
    m_PhysicalDevice = physicalDevices[0];
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos =
        GetDeviceQueueCreateInfos(m_PhysicalDevice);
    std::vector<const char*> deviceExtensions =
        GetDeviceExtentionNames(m_PhysicalDevice);

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount =
            static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()};

    result =
        vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
    LogVulkanError("Could not create vulkan logical device", result);
}

void Video::CreateSwapchain()
{
    VkResult result;
    m_SurfaceFormat = GetSurfaceFormat();
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_Surface,
        .minImageCount = m_SurfaceCapabilities.minImageCount + 1,
        .imageFormat = m_SurfaceFormat.format,
        .imageColorSpace = m_SurfaceFormat.colorSpace,
        .imageExtent = m_SurfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = true};

    result = vkCreateSwapchainKHR(
        m_Device, &swapchainCreateInfo, nullptr, &m_Swapchain);
    LogVulkanError("Failed to create swapchain", result);
}

void Video::CreateSwapchainImageViews()
{
    VkResult result;
    std::vector<VkImage> swapchainImages = GetSwapchainImages();
    m_SwapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        VkImageView& swapchainImageView = m_SwapchainImageViews.at(i);
        VkImageViewCreateInfo swapchainImageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_SurfaceFormat.format,
            .components =
                {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}};
        result = vkCreateImageView(
            m_Device, &swapchainImageViewCreateInfo, nullptr,
            &swapchainImageView);
        LogVulkanError("Failed to create swapchain image view", result);
    }
}

// ugly ugly ugly
std::vector<VkPipelineShaderStageCreateInfo> Video::CreatePipelineShaderStage()
{
    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = m_Shaders.at(0).vertShaderModule,
        .pName = "main"};

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = m_Shaders.at(0).fragShaderModule,
        .pName = "main"};

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
        vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    return shaderStages;
}

void Video::CreateRenderPass()
{
    VkResult result;
    VkAttachmentDescription colorAttachment = {
        .format = m_SurfaceFormat.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };
    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass};
    result = vkCreateRenderPass(
        m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass);
    LogVulkanError("failed to create render pass", result);
}

void Video::CreateGraphicsPipeline()
{
    VkResult result;
    m_Shaders = LoadShaders(m_Device);
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
        CreatePipelineShaderStage();

    VkVertexInputBindingDescription bindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    std::array<VkVertexInputAttributeDescription, 2> attributeDescription = {
        {{.location = 0,
          .binding = 0,
          .format = VK_FORMAT_R32G32_SFLOAT,
          .offset = offsetof(Vertex, pos)},
         {.location = 1,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32_SFLOAT,
          .offset = offsetof(Vertex, color)}}};

    VkPipelineVertexInputStateCreateInfo vertexInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescription.size()),
        .pVertexAttributeDescriptions = attributeDescription.data()};

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_SurfaceCapabilities.currentExtent.width),
        .height =
            static_cast<float>(m_SurfaceCapabilities.currentExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f};

    VkRect2D scissor = {
        .offset = {0, 0}, .extent = m_SurfaceCapabilities.currentExtent};

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    VkPipelineRasterizationStateCreateInfo rasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE};

    VkStencilOpState frontStencil{};
    VkStencilOpState backStencil{};

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable =
            VK_FALSE, // always false when depthTestEnable is false
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .stencilTestEnable = VK_FALSE,
        .front = frontStencil,
        .back = backStencil,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 100.0f};

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState};

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .layout = m_PipelineLayout,
        .renderPass = m_RenderPass,
        .subpass = 0};

    result = vkCreateGraphicsPipelines(
        m_Device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr,
        &m_Pipeline);
    LogVulkanError("failed to create graphics pipeline", result);
}

void Video::CreateFramebuffers()
{
    VkResult result;
    assert(m_SwapchainImageViews.size() > 0);
    m_Framebuffers.resize(m_SwapchainImageViews.size());
    for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
    {
        VkFramebuffer& framebuffer = m_Framebuffers.at(i);
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_RenderPass,
            .attachmentCount = 1,
            .pAttachments = &m_SwapchainImageViews.at(i),
            .width = m_SurfaceCapabilities.currentExtent.width,
            .height = m_SurfaceCapabilities.currentExtent.height,
            .layers = 1};

        result = vkCreateFramebuffer(
            m_Device, &framebufferCreateInfo, nullptr, &framebuffer);
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
        m_Device, &commandPoolCreateInfo, nullptr, &m_CommandPool);
    LogVulkanError("failed to create command pool", result);

    VkCommandBufferAllocateInfo commandBuffersAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};
    result = vkAllocateCommandBuffers(
        m_Device, &commandBuffersAllocateInfo, &m_CommandBuffer);
    LogVulkanError("failed to allocate command buffers", result);
}

void Video::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    CreateBuffer(
        bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_VertexBufferMemory, m_VertexBuffer);

    // load hard-coded vertices into memory
    void* data;
    vkMapMemory(m_Device, m_VertexBufferMemory, 0, bufferSize, 0, &data);
    std::span<Vertex> memorySpan(static_cast<Vertex*>(data), vertices.size());
    std::copy_n(vertices.begin(), vertices.size(), memorySpan.begin());
    vkUnmapMemory(m_Device, m_VertexBufferMemory);
}

void Video::CreateUniformBuffers()
{
    VkResult result;
    m_UniformBuffers.resize(m_SwapchainImageViews.size());
    m_UniformBufferMemoryMapped.resize(m_SwapchainImageViews.size());
    m_UniformBufferMemory.resize(m_SwapchainImageViews.size());
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    for (size_t i = 0; i < m_UniformBuffers.size(); i++)
    {
        CreateBuffer(
            bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_UniformBufferMemory.at(i), m_UniformBuffers.at(i));

        result = vkMapMemory(
            m_Device, m_UniformBufferMemory.at(i), 0, bufferSize, 0,
            &m_UniformBufferMemoryMapped.at(i));
        LogVulkanError("failed to map uniform buffer memory", result);
    }
}

void Video::CreateDescriptorPool()
{
    VkResult result;
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t>(m_SwapchainImageViews.size())};

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize};

    poolInfo.maxSets = static_cast<uint32_t>(m_SwapchainImageViews.size());
    result =
        vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool);
    LogVulkanError("Failed to create descriptor pool", result);
}

void Video::CreateDescriptorSets()
{
    VkResult result;
    std::vector<VkDescriptorSetLayout> layouts(
        m_SwapchainImageViews.size(), m_DescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_DescriptorPool,
        .descriptorSetCount =
            static_cast<uint32_t>(m_SwapchainImageViews.size()),
        .pSetLayouts = layouts.data()};

    m_DescriptorSets.resize(m_SwapchainImageViews.size());
    result =
        vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data());

    for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = m_UniformBuffers.at(i),
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

        vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
    }
}

void Video::CreateMemoryBarriers()
{
    VkResult result;
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    result = vkCreateSemaphore(
        m_Device, &semaphoreCreateInfo, nullptr, &m_Semaphore);
    LogVulkanError("failed to create semaphore", result);
    result = vkCreateSemaphore(
        m_Device, &semaphoreCreateInfo, nullptr, &m_RenderSemaphore);
    LogVulkanError("failed to create semaphore", result);
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    result = vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Fence);
    LogVulkanError("failed to create fence", result);
}

std::vector<const char*> Video::GetExtensionNames()
{
    std::vector<const char*> extNames = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

    std::vector<const char*> windowExtNames =
        m_Window.GetRequiredExtensionNames();
    extNames.insert(
        extNames.end(), windowExtNames.begin(), windowExtNames.end());

    std::vector<VkExtensionProperties> instanceSupportedExtensions =
        GetInstanceSupportedExtensions();

    if (std::find_if(
            instanceSupportedExtensions.begin(),
            instanceSupportedExtensions.end(),
            [](VkExtensionProperties& properties)
            {
                return !strcmp(
                    properties.extensionName,
                    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }) != instanceSupportedExtensions.end())
    {
        extNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }

    for (const char* extName : extNames)
    {
        if (std::find_if(
                instanceSupportedExtensions.begin(),
                instanceSupportedExtensions.end(),
                [extName](VkExtensionProperties& properties) {
                    return !strcmp(properties.extensionName, extName);
                }) == instanceSupportedExtensions.end())
        {
            LogError(
                fmt::format("Required extension {} not supported!", extName));
        }
    }

    LogDebug("Enabled exensions:");
    for (const char* extName : extNames)
    {
        LogDebug(fmt::format("\t{}", extName));
    }
    return extNames;
}

std::vector<VkExtensionProperties> Video::GetInstanceSupportedExtensions()
{
    VkResult result;

    uint32_t instanceSupportedExtensionCount;
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount, nullptr);
    LogVulkanError("Failed to get instance supported extension count", result);

    std::vector<VkExtensionProperties> instanceSupportedExtensions(
        instanceSupportedExtensionCount);
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount,
        instanceSupportedExtensions.data());
    LogVulkanError("Failed to get instance supported extension count", result);

    LogDebug("Instance Supported Extensions:");
    for (const VkExtensionProperties& properties : instanceSupportedExtensions)
    {
        LogDebug(fmt::format("\t{}", properties.extensionName));
    }
    return instanceSupportedExtensions;
}

std::vector<VkPhysicalDevice> Video::GetPhysicalDevices()
{
    VkResult result;
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        LogError("No vukan devices found!");
    }
    LogVulkanError("Could not fetch physical device count", result);

    std::vector<VkPhysicalDevice> physicalDevices(
        static_cast<size_t>(deviceCount));
    result = vkEnumeratePhysicalDevices(
        m_Instance, &deviceCount, &physicalDevices.front());
    LogVulkanError("Problem enumerating physical devices", result);

    LogDebug("Physical Devices:");
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices.at(i), &properties);
        LogDebug(fmt::format(
            "\tPhysical device [{}]: {}", i, properties.deviceName));
    }

    return physicalDevices;
}

std::vector<VkDeviceQueueCreateInfo>
Video::GetDeviceQueueCreateInfos(VkPhysicalDevice physicalDevice)
{
    VkResult result;
    uint32_t queueFamilyPropertyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0)
    {
        LogError("No physical device queues found");
    }
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount,
        queueFamilyProperties.data());

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(
        static_cast<size_t>(queueFamilyPropertyCount));

    m_DeviceQueues.resize(
        queueFamilyPropertyCount, {{std::vector<float>{1.0f}}});

    LogDebug("Queues:");
    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++)
    {
        const VkQueueFamilyProperties& properties = queueFamilyProperties.at(i);

        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice, i, m_Surface,
            &m_DeviceQueues.at(i).m_PresentationSupported);
        LogVulkanError(
            fmt::format(
                "Failed to check whether queue {} supports presentation", i),
            result);

        LogDebug(fmt::format(
            "\tQueue Family [{}]: {:#b} {} {}", i, properties.queueFlags,
            properties.queueCount,
            static_cast<bool>(m_DeviceQueues.at(i).m_PresentationSupported)));

        VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = 1,
            .pQueuePriorities = m_DeviceQueues.at(i).m_Priorities.data()};
        deviceQueueCreateInfos.at(i) = (deviceQueueCreateInfo);
    }
    return deviceQueueCreateInfos;
}

std::vector<const char*>
Video::GetDeviceExtentionNames(VkPhysicalDevice physicalDevice)
{
    VkResult result;
    uint32_t deviceSupportedExtensionsCount = 0;
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount, nullptr);
    LogVulkanError("error getting available device extension count", result);

    std::vector<VkExtensionProperties> deviceSupportedExtensions(
        static_cast<size_t>(deviceSupportedExtensionsCount));
    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount,
        deviceSupportedExtensions.data());
    LogVulkanError("error getting available device extensions", result);

    LogDebug("Physical Device Supported Extensions:");
    for (const auto properties : deviceSupportedExtensions)
    {
        LogDebug(fmt::format("\t{}", properties.extensionName));
    }

    for (const auto& extension : deviceSupportedExtensions)
    {
        // https://vulkan.lunarg.com/doc/view/1.3.250.1/mac/1.3-extensions/vkspec.html#VUID-VkDeviceCreateInfo-pProperties-04451
        if (!strcmp(
                extension.extensionName,
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        {
            deviceExtensions.push_back(
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
        }
    }

    return deviceExtensions;
}

VkSurfaceCapabilitiesKHR Video::GetSurfaceCapabilities()
{
    VkResult result;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_PhysicalDevice, m_Surface, &surfaceCapabilities);

    LogDebug(fmt::format(
        "Surface capabiltiies:\t{}", surfaceCapabilities.currentExtent));
    return surfaceCapabilities;
}

VkSurfaceFormatKHR Video::GetSurfaceFormat()
{
    return PickSurfaceFormat(
        GetCompatableSurfaceFormats(), VK_FORMAT_B8G8R8A8_UNORM);
}

const std::vector<VkSurfaceFormatKHR> Video::GetCompatableSurfaceFormats()
{
    VkResult result;
    uint32_t surfaceFormatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_PhysicalDevice, m_Surface, &surfaceFormatCount, nullptr);
    LogVulkanError("Failed to get supported surface format count", result);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(
        static_cast<size_t>(surfaceFormatCount));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_PhysicalDevice, m_Surface, &surfaceFormatCount,
        surfaceFormats.data());
    LogVulkanError(
        "Failed to get surface formats supported by the device", result);

    LogDebug("Supported Formats:");
    for (const auto& surfaceFormat : surfaceFormats)
    {
        LogDebug(fmt::format(
            "\t{}, {}", surfaceFormat.colorSpace, surfaceFormat.format));
    }
    return surfaceFormats;
}

VkSurfaceFormatKHR Video::PickSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& surfaceFormats,
    const VkFormat requestedFormat)
{
    VkSurfaceFormatKHR surfaceFormat = {
        .format = VK_FORMAT_UNDEFINED,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    if (surfaceFormats.size() == 1 &&
        surfaceFormats.at(0).format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR};
    }
    else if (
        std::find_if(
            surfaceFormats.begin(), surfaceFormats.end(),
            [requestedFormat](VkSurfaceFormatKHR surfaceFormat) {
                return surfaceFormat.format == requestedFormat;
            }) != surfaceFormats.end())
    {
        surfaceFormat.format = requestedFormat;
    }

    // fallback
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = surfaceFormats.at(0);
        LogWarning(fmt::format(
            "Requested format not found! Falling back to: {} {}",
            surfaceFormat.format, surfaceFormat.colorSpace));
    }
    return surfaceFormat;
}

std::vector<VkImage> Video::GetSwapchainImages()
{
    VkResult result;
    uint32_t swapchainImageCount = 0;
    result = vkGetSwapchainImagesKHR(
        m_Device, m_Swapchain, &swapchainImageCount, nullptr);
    LogVulkanError("Failed to get swapchain image count", result);
    std::vector<VkImage> swapchainImages(
        static_cast<size_t>(swapchainImageCount));
    result = vkGetSwapchainImagesKHR(
        m_Device, m_Swapchain, &swapchainImageCount, swapchainImages.data());
    LogVulkanError("Failed to get swapchain images", result);
    return swapchainImages;
}

void Video::CreatePipelineLayout()
{
    VkResult result;
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};
    descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = descriptorSetLayoutBindings.data()};
    result = vkCreateDescriptorSetLayout(
        m_Device, &descriptorSetLayoutCreateInfo, nullptr,
        &m_DescriptorSetLayout);
    LogVulkanError("failed to create descriptor set layout", result);
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &m_DescriptorSetLayout};
    result = vkCreatePipelineLayout(
        m_Device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout);
    LogVulkanError("failed to create pipeline layout", result);
}

VkQueue Video::GetQueue(uint32_t queueFamily, uint32_t index)
{
    VkQueue queue;
    vkGetDeviceQueue(m_Device, index, 0, &queue);
    return queue;
}

VkInstanceCreateFlags
Video::GetInstanceCreateFlags(const std::vector<const char*>& extentionNames)
{
    VkInstanceCreateFlags instanceCreateFlags = {};
    if (std::find_if(
            extentionNames.begin(), extentionNames.end(),
            [](const char* extName) {
                return !strcmp(
                    extName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }) != extentionNames.end())
    {
        instanceCreateFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    return instanceCreateFlags;
}

uint32_t Video::FindMemoryType(
    VkMemoryRequirements memoryRequirements,
    VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memoryProperties);

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
            (memoryProperties.memoryTypes[i].propertyFlags &
             memoryPropertyFlags) == memoryPropertyFlags)
        {
            memoryTypeIndex = i;
            break;
        }
        if (i == memoryProperties.memoryTypeCount - 1)
        {
            LogError("ERROR: failed to find suitable memory type\n");
        }
    }
    return memoryTypeIndex;
}

void Video::CreateBuffer(
    VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory& bufferMemory,
    VkBuffer& buffer)
{
    VkResult result;
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    result = vkCreateBuffer(m_Device, &bufferCreateInfo, nullptr, &buffer);
    LogVulkanError("failed to create vertex buffer", result);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memoryRequirements);
    uint32_t memoryTypeIndex =
        FindMemoryType(memoryRequirements, memoryPropertyFlags);

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex};

    result =
        vkAllocateMemory(m_Device, &memoryAllocateInfo, nullptr, &bufferMemory);
    LogVulkanError("failed to allocate buffer memory", result);

    result = vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
    LogVulkanError("failed to bind buffer memory", result);
}
