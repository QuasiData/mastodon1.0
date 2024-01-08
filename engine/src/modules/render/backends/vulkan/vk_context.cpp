#include "vk_context.h"
#include "vk_debug.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

namespace mas::gfx::vulkan
{
namespace
{
#if _DEBUG
bool enable_validation{ true };
#else
bool enable_validation{ true };
#endif
}
Context::Context(GLFWwindow* w)
{
    spdlog::info("Initialising vulkan");
    if (const auto result = volkInitialize(); !result)
        throw std::runtime_error("Failed to initialise volk");

    window = w;
    instance = create_instance();
    // volkloadinstance

    // enable debug layers

    // init surface
    // create the device

    // volkloaddevice

    // get function pointer for vma
    // create vma allocator

    // create the swap chain
}

VkInstance Context::create_instance()
{
    VkApplicationInfo app_info{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app_info.pApplicationName = "Mastodon";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.pEngineName = "Mastodon Engine";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion = VK_API_VERSION_1_3;

    u32 glfw_ext_count{ 0 };
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

    u32 supported_ext_count{ 0 };
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_count, nullptr);
    std::vector<VkExtensionProperties> supported_exts(supported_ext_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_ext_count, supported_exts.data());

    for (usize i{ 0 }; i < glfw_ext_count; ++i)
    {
        bool exists{ false };
        for (const auto& [ext_name, _] : supported_exts)
        {
            if (strcmp(ext_name, glfw_extensions[i]) == 0)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
            throw std::runtime_error("Failed to find extensions required by glfw!");
    }

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_ext_count);

    if constexpr (enable_validation)
    {
        spdlog::info("Vulkan validation layer enabled");
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    std::vector<const char*> layers{};

    if constexpr (enable_validation)
    {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    VkInstanceCreateInfo instance_ci{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instance_ci.pApplicationInfo = &app_info;
    instance_ci.flags = 0;

    instance_ci.pNext = nullptr;
    VkDebugUtilsMessengerCreateInfoEXT du_ci{};
    debug::populate_ci(du_ci);
    if constexpr (enable_validation)
    {
        instance_ci.pNext = &du_ci;
    }

    instance_ci.enabledExtensionCount = static_cast<u32>(extensions.size());
    instance_ci.ppEnabledExtensionNames = extensions.data();
    instance_ci.enabledLayerCount = static_cast<u32>(layers.size());
    instance_ci.ppEnabledLayerNames = layers.data();

    VkInstance instance{};

    if (vkCreateInstance(&instance_ci, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create instance!");

    return instance;
}
}
