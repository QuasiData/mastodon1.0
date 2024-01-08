#pragma once
#include <volk/volk.h>

namespace mas::gfx::vulkan::debug
{
void populate_ci(VkDebugUtilsMessengerCreateInfoEXT& ci);
void init(VkInstance instance);
void shutdown(VkInstance instance);
}
