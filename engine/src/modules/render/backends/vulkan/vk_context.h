#pragma once
#include "common.h"

#include "volk/volk.h"
#include "vma/vk_mem_alloc.h"

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

namespace mas::gfx::vulkan
{
class Context
{
public:
    explicit Context(GLFWwindow* w);

private:
    static VkInstance create_instance();

    GLFWwindow* window{ nullptr };
    VkInstance instance{ nullptr };
};
}