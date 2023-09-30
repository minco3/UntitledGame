#include <vector>

class DeviceQueue
{
public:
    DeviceQueue(const std::vector<float>& priorities);

    std::vector<float> m_Priorities;
};