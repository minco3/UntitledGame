#include "DeviceQueue.hpp"

DeviceQueue::DeviceQueue(
    const std::vector<float>& priorities, vk::Bool32 presentationSupported)
    : m_Priorities(priorities), m_PresentationSupported(presentationSupported)
{
}