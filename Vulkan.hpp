#pragma once

#include <vulkan/vulkan.h>

class Vulkan
{
public:
    Vulkan& Instance() {
        static Vulkan instance;
        return instance;
    }
private:
    Vulkan();
    ~Vulkan();
    void createInstance();
    VkInstance m_instance;
};