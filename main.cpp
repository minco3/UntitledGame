#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_vulkan.h>
#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#define VMA_IMPLEMENTATION
#include "vulkanfmt.h"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

// #define DEBUG

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

void LogError(const std::string& message, VkResult result)
{
    if (result != VK_SUCCESS)
    {
        fmt::println(std::cerr, "ERROR: {}: {}", message, result);
        exit(1);
    }
}

int main()
{

    VkResult result;
    bool running = true;
    SDL_Window* window;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fmt::println(
            std::cerr, "ERROR: error initializing sdl: {}", SDL_GetError());
    }

    window = SDL_CreateWindow(
        "SDL2 VULKAN TEST", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1200, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (window == nullptr)
    {
        fmt::println(
            std::cerr, "ERROR: error creating SDL window: ", SDL_GetError());
    }

    std::vector<const char*> extNames = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME};

    std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    unsigned int numExtentions;
    if (SDL_Vulkan_GetInstanceExtensions(window, &numExtentions, nullptr) !=
        SDL_TRUE)
    {
        fmt::println(
            std::cerr,
            "ERROR: error getting SDL required vulkan exension count");
    }
    std::vector<const char*> sdlExtNames(numExtentions);
    if (SDL_Vulkan_GetInstanceExtensions(
            window, &numExtentions, sdlExtNames.data()) != SDL_TRUE)
    {
        fmt::println(
            std::cerr, "ERROR: error getting SDL required vulkan extensions");
    }

    extNames.insert(extNames.end(), sdlExtNames.begin(), sdlExtNames.end());

    uint32_t instanceSupportedExtensionCount;
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount, nullptr);
    LogError("Failed to get instance supported extension count", result);

    std::vector<VkExtensionProperties> instanceSupportedExtensions(
        instanceSupportedExtensionCount);
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount,
        instanceSupportedExtensions.data());
    LogError("Failed to get instance supported extension count", result);

#ifdef DEBUG
    fmt::println(std::cout, "Instance Supported Extensions:");
    for (const VkExtensionProperties& properties : instanceSupportedExtensions)
    {
        fmt::println(std : cout, "\t{}", properties.extensionName);
    }
#endif
    for (const char* extName : extNames)
    {
        for (uint32_t i = 0; i < instanceSupportedExtensionCount; i++)
        {
            const VkExtensionProperties& properties =
                instanceSupportedExtensions.at(i);
            if (!strcmp(extName, properties.extensionName))
            {
                break;
            }
            else if (i == instanceSupportedExtensionCount - 1)
            {
                fmt::println(
                    std::cerr, "ERROR: Requested extension not supported!");
                return 1;
            }
        }
    }

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "SDL TEST",
        .applicationVersion = 1,
        .pEngineName = nullptr,
        .apiVersion = VK_API_VERSION_1_3};

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extNames.size()),
        .ppEnabledExtensionNames = extNames.data()};

    VkInstance instance;

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE)
    {
        fmt::println(std::cerr, "ERROR: error creating surface");
        exit(1);
    }

    LogError("error creating vulkan instance", result);

    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        fmt::println(std::cerr, "ERROR: no vukan devices found!");
        exit(1);
    }
    LogError("error fetching physical device count", result);

    std::vector<VkPhysicalDevice> physicalDevices(
        static_cast<size_t>(deviceCount));
    result = vkEnumeratePhysicalDevices(
        instance, &deviceCount, &physicalDevices.front());
    LogError("error enumerating physical devices", result);

    fmt::println(std::cout, "Physical Devices:");
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices.at(i), &properties);
        fmt::println(
            std::cout, "\tPhysical device [{}]: {}", i, properties.deviceName);
    }

    VkPhysicalDevice physicalDevice = physicalDevices[0];

    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0)
    {
        fmt::println(std::cerr, "ERROR: no physical device queues found");
        exit(1);
    }
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount,
        queueFamilyProperties.data());

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(
        static_cast<size_t>(queueFamilyPropertyCount));
    const float priority = 1.0f;

    fmt::println(std::cout, "Queues:");
    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++)
    {
        const VkQueueFamilyProperties& properties = queueFamilyProperties.at(i);

        VkBool32 supported;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice, i, surface, &supported);
        LogError("failed to get physical device support", result);

        fmt::println(
            std::cout, "\tQueue Family [{}]: {:#b} {} {}", i,
            properties.queueFlags, properties.queueCount,
            static_cast<bool>(supported));

        VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = 1,
            .pQueuePriorities = &priority};
        deviceQueueCreateInfos.at(i) = (deviceQueueCreateInfo);
    }

    uint32_t deviceSupportedExtensionsCount = 0;
    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount, nullptr);
    LogError("error getting available device extension count", result);

    std::vector<VkExtensionProperties> deviceSupportedExtensions(
        static_cast<size_t>(deviceSupportedExtensionsCount));
    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount,
        deviceSupportedExtensions.data());
    LogError("error getting available device extensions", result);

#ifdef DEBUG
    fmt::println(std::cout, "Physical Device Supported Extensions:");
    for (const auto properties : deviceSupportedExtensions)
    {
        fmt::println("\t{}", properties.extensionName);
    }
#endif

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

    uint32_t surfaceFormatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &surfaceFormatCount, nullptr);
    LogError("Failed to get physical device format count", result);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(
        static_cast<size_t>(surfaceFormatCount));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());
    LogError("Failed to get physical device formats", result);

#ifdef DEBUG
    fmt::println(std::cout, "Supported Formats:");
    for (const auto& surfaceFormat : surfaceFormats)
    {
        fmt::println(
            std::cout, "\t{}, {}", surfaceFormat.colorSpace,
            surfaceFormat.format);
    }
#endif

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice, surface, &surfaceCapabilities);

    fmt::println(
        std::cout, "Surface capabiltiies:\n\t{}",
        surfaceCapabilities.currentExtent);

    VkDevice device = {};
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount =
            static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()};

    result =
        vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        LogError("error creating device", result);
        exit(1);
    }

    uint32_t numImages = surfaceCapabilities.minImageCount + 1;

    VkSurfaceFormatKHR surfaceFormat = {.format = VK_FORMAT_UNDEFINED};
    if (surfaceFormats.size() == 1 &&
        surfaceFormats.at(0).format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR};
    }
    else
    {
        for (const VkSurfaceFormatKHR& format : surfaceFormats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                surfaceFormat = format;
                break;
            }
        }
    }
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = surfaceFormats.at(0);
        fmt::println(
            std::cerr,
            "WARNING: Requested format not found! falling back to: {} {}",
            surfaceFormat.format, surfaceFormat.colorSpace);
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = numImages,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = surfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = true};

    VkSwapchainKHR swapchain;
    result =
        vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    LogError("failed to create swapchain", result);

    uint32_t swapchainImageCount = 0;
    result = vkGetSwapchainImagesKHR(
        device, swapchain, &swapchainImageCount, nullptr);
    LogError("failed to get swapchain image count", result);
    std::vector<VkImage> swapchainImages(
        static_cast<size_t>(swapchainImageCount));
    result = vkGetSwapchainImagesKHR(
        device, swapchain, &swapchainImageCount, swapchainImages.data());
    LogError("Failed to get swapchain images", result);

    std::vector<VkImageView> swapchainImageViews(swapchainImages.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        VkImageView& swapchainImageView = swapchainImageViews.at(i);
        VkImageViewCreateInfo swapchainImageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surfaceFormat.format,
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
            device, &swapchainImageViewCreateInfo, nullptr,
            &swapchainImageView);
        LogError("failed to create swapchain image view", result);
    }

    std::ifstream vertShaderFile("shader.vert.spv", std::ios::binary);
    if (!vertShaderFile.is_open())
    {
        fmt::println(std::cerr, "ERROR: failed to open the vertex shader file");
        exit(1);
    }
    std::string vertCode(
        (std::istreambuf_iterator<char>(vertShaderFile)),
        std::istreambuf_iterator<char>());

    std::ifstream fragShaderFile("shader.frag.spv", std::ios::binary);
    if (!fragShaderFile.is_open())
    {
        fmt::println(
            std::cerr, "ERROR: failed to open the fragment shader file");
        exit(1);
    }
    std::string fragCode(
        (std::istreambuf_iterator<char>(fragShaderFile)),
        std::istreambuf_iterator<char>());

    VkShaderModuleCreateInfo vertShaderCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vertCode.size(),
        .pCode = reinterpret_cast<const uint32_t*>(vertCode.data())};
    VkShaderModuleCreateInfo fragShaderCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fragCode.size(),
        .pCode = reinterpret_cast<const uint32_t*>(fragCode.data())};

    VkShaderModule fragShader;
    VkShaderModule vertShader;

    result = vkCreateShaderModule(
        device, &vertShaderCreateInfo, nullptr, &vertShader);
    LogError("failed to create vertex shader", result);
    result = vkCreateShaderModule(
        device, &fragShaderCreateInfo, nullptr, &fragShader);
    LogError("failed to create fragment shader", result);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShader,
        .pName = "main"};

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShader,
        .pName = "main"};

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
        vertShaderStageCreateInfo, fragShaderStageCreateInfo};

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

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(surfaceCapabilities.currentExtent.width),
        .height = static_cast<float>(surfaceCapabilities.currentExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f};

    VkRect2D scissor = {
        .offset = {0, 0}, .extent = surfaceCapabilities.currentExtent};

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

    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    result = vkCreatePipelineLayout(
        device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    LogError("failed to create pipeline layout", result);

    VkRenderPass renderPass;

    VkAttachmentDescription colorAttachment = {
        .format = surfaceFormat.format,
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
    result =
        vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
    LogError("failed to create render pass", result);

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
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0};

    VkPipeline pipeline;
    result = vkCreateGraphicsPipelines(
        device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr,
        &pipeline);
    LogError("failed to create graphics pipeline", result);

    std::vector<VkFramebuffer> framebuffers(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        VkFramebuffer& framebuffer = framebuffers.at(i);
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = 1,
            .pAttachments = &swapchainImageViews.at(i),
            .width = surfaceCapabilities.currentExtent.width,
            .height = surfaceCapabilities.currentExtent.height,
            .layers = 1};

        result = vkCreateFramebuffer(
            device, &framebufferCreateInfo, nullptr, &framebuffer);
        LogError("failed to create swapchain", result);
    }

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
    LogError("failed to create command pool", result);

    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo commandBuffersAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};
    result = vkAllocateCommandBuffers(
        device, &commandBuffersAllocateInfo, &commandBuffer);
    LogError("failed to allocate command buffers", result);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT};

    VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkClearValue clearValue = {.color = clearColor};

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    VkBuffer vertexBuffer;
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vertices.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &vertexBuffer);
    LogError("failed to create vertex buffer", result);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t memoryTypeIndex;

    fmt::println(
        std::cout, "TypeFilter: {:#b}", memoryRequirements.memoryTypeBits);

    fmt::println(std::cout, "Available Memory Types:");
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        fmt::println(
            std::cout, "\t{:#b}",
            static_cast<uint32_t>(
                memoryProperties.memoryTypes[i].propertyFlags));
    }

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memoryTypeIndex = i;
            break;
        }
        if (i == memoryProperties.memoryTypeCount - 1)
        {
            fmt::println(
                std::cerr, "ERROR: failed to find suitable memory type");
            exit(1);
        }
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex};

    VkDeviceMemory vertexBufferMemory;

    result = vkAllocateMemory(
        device, &memoryAllocateInfo, nullptr, &vertexBufferMemory);
    LogError("failed to allocate vertex buffer memory", result);

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
    LogError("failed to create semaphore", result);
    VkSemaphore renderSemaphore = {};
    result = vkCreateSemaphore(
        device, &semaphoreCreateInfo, nullptr, &renderSemaphore);
    LogError("failed to create semaphore", result);
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    result = vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
    LogError("failed to create fence", result);

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
        LogError("failed to aquire next image", result);

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
        LogError("failed to start recording command buffer", result);

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
        LogError("failed to stop recording command buffer", result);

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
        LogError("failed to submit draw command buffer", result);

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIndex};
        result = vkQueuePresentKHR(queue, &presentInfo);
        LogError("failed to present queue", result);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}