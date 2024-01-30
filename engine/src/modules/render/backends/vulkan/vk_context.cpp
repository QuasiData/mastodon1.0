// ReSharper disable CppClangTidyBugproneUncheckedOptionalAccess
#include "vk_context.h"
#include "vk_debug.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

#include <set>

namespace mas::gfx::vulkan
{
namespace
{
bool is_device_suitable(const VkPhysicalDevice physical_device, const std::vector<const char*>& req_extensions)
{
    VkPhysicalDeviceProperties2 dp{};
    dp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2(physical_device, &dp);

    u32 ext_count{ 0 };
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, nullptr);
    std::vector<VkExtensionProperties> extension(ext_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, extension.data());

    for (const char* req_ext_name : req_extensions)
    {
        bool found{ false };
        for (const auto& [ext_name, _] : extension)
        {
            if (strcmp(ext_name, req_ext_name) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    return true;
}

QueueFamilyIndices find_queue_families(const VkPhysicalDevice p, const VkSurfaceKHR surface)
{
    assert(p);
    assert(surface);

    QueueFamilyIndices indices;
    u32 qf_count{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(p, &qf_count, nullptr);
    std::vector<VkQueueFamilyProperties> q_families(qf_count);
    vkGetPhysicalDeviceQueueFamilyProperties(p, &qf_count, q_families.data());

    for (u32 i{ 0 }; i < qf_count; ++i)
    {
        if (q_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
            break;
        }
    }

    for (u32 i{ 0 }; i < qf_count; ++i)
    {
        VkBool32 present_support{ false };
        vkGetPhysicalDeviceSurfaceSupportKHR(p, i, surface, &present_support);

        if (present_support)
        {
            indices.present_family = i;
            break;
        }
    }

    return indices;
}
}
Context::Context(GLFWwindow* w)
{
    spdlog::info("Initialising vulkan");
    if (const auto result = volkInitialize(); result != VK_SUCCESS)
        throw std::runtime_error("Failed to initialise volk");

    window = w;
    instance = create_instance();
    volkLoadInstance(instance);

    if constexpr (enable_validation)
        debug::init(instance);

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create glfw window surface!");

    create_device();
    volkLoadDevice(device);

    VmaVulkanFunctions vma_vulkan_func{};
    vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
    vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
    vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
    vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
    vma_vulkan_func.vkCreateImage = vkCreateImage;
    vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
    vma_vulkan_func.vkDestroyImage = vkDestroyImage;
    vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vma_vulkan_func.vkFreeMemory = vkFreeMemory;
    vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vma_vulkan_func.vkMapMemory = vkMapMemory;
    vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
    vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo alloc_ci{};
    alloc_ci.instance = instance;
    alloc_ci.device = device;
    alloc_ci.physicalDevice = phys_device;
    alloc_ci.vulkanApiVersion = VK_API_VERSION_1_3;
    alloc_ci.pVulkanFunctions = &vma_vulkan_func;

    vmaCreateAllocator(&alloc_ci, &allocator);

    finalize_swapchain();
}

Context::~Context()
{
    for (const auto image_view : surface_image_views)
        vkDestroyImageView(device, image_view, nullptr);

    vkDestroySwapchainKHR(device, swap_chain, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vmaDestroyAllocator(allocator);

    vkDestroyDevice(device, nullptr);

    if constexpr (enable_validation)
        debug::shutdown(instance);

    vkDestroyInstance(instance, nullptr);
}

void Context::resize_swapchain()
{
    finalize_swapchain();
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

void Context::create_device()
{
    u32 phys_device_count{ 0 };
    vkEnumeratePhysicalDevices(instance, &phys_device_count, nullptr);
    assert(phys_device_count > 0);
    std::vector<VkPhysicalDevice> physical_devices(phys_device_count);
    vkEnumeratePhysicalDevices(instance, &phys_device_count, physical_devices.data());

    const std::vector<const char*> req_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    phys_device = physical_devices[0];
    for (usize i{ 0 }; i < physical_devices.size(); ++i)
    {
        if (is_device_suitable(physical_devices[i], req_extensions))
        {
            VkPhysicalDeviceProperties2 dp{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
            phys_device = physical_devices[i];
            vkGetPhysicalDeviceProperties2(phys_device, &dp);

            if (const auto qf = find_queue_families(phys_device, surface);
                dp.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && qf.is_complete())
            {
                spdlog::info("Found compatible device: {}", dp.properties.deviceName);
                break;
            }
        }
    }

    queue_family_indices = find_queue_families(phys_device, surface);
    std::set<u32> unique_qf = { queue_family_indices.graphics_family.value(), queue_family_indices.present_family.value() };

    std::vector<VkDeviceQueueCreateInfo> queue_cis;
    constexpr f32 queue_priority{ 1.0f };
    for (u32 q_f : unique_qf)
    {
        VkDeviceQueueCreateInfo q_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        q_info.queueFamilyIndex = q_f;
        q_info.queueCount = 1;
        q_info.pQueuePriorities = &queue_priority;
        queue_cis.push_back(q_info);
    }

    auto syn2 = VkPhysicalDeviceSynchronization2Features{};
    syn2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    syn2.pNext = nullptr;
    auto dyn_render = VkPhysicalDeviceDynamicRenderingFeatures{};
    dyn_render.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dyn_render.pNext = &syn2;
    auto desc_index = VkPhysicalDeviceDescriptorIndexingFeatures{};
    desc_index.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    desc_index.pNext = &dyn_render;
    auto buffer_device_address = VkPhysicalDeviceBufferDeviceAddressFeatures{};
    buffer_device_address.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    buffer_device_address.pNext = &desc_index;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &buffer_device_address;

    vkGetPhysicalDeviceFeatures2(phys_device, &features2);

    if (syn2.synchronization2 != VkBool32{ 1 })
        throw std::runtime_error("Device does not support features required by vulkan renderer");
    if (dyn_render.dynamicRendering != VkBool32{ 1 })
        throw std::runtime_error("Device does not support features required by vulkan renderer");
    if (desc_index.descriptorBindingPartiallyBound != VkBool32{ 1 })
        throw std::runtime_error("Device does not support features required by vulkan renderer");
    if (buffer_device_address.bufferDeviceAddress != VkBool32{ 1 })
        throw std::runtime_error("Device does not support features required by vulkan renderer");

    VkPhysicalDeviceFeatures enabled_features{};

    if (features2.features.samplerAnisotropy)
        enabled_features.samplerAnisotropy = 1;

    if (features2.features.sparseBinding)
        enabled_features.sparseBinding = 1;

    VkDeviceCreateInfo device_ci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    device_ci.pQueueCreateInfos = queue_cis.data();
    device_ci.queueCreateInfoCount = static_cast<u32>(queue_cis.size());
    device_ci.pEnabledFeatures = nullptr;
    device_ci.ppEnabledExtensionNames = req_extensions.data();
    device_ci.enabledExtensionCount = static_cast<u32>(req_extensions.size());
    device_ci.pNext = &features2;

    if (vkCreateDevice(phys_device, &device_ci, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    vkGetDeviceQueue(device, queue_family_indices.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(device, queue_family_indices.present_family.value(), 0, &present_queue);

}

void Context::finalize_swapchain()
{
    assert(phys_device);
    assert(device);

    auto old_swap_chain = swap_chain;

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};
    };

    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &details.capabilities);

    u32 format_count{ 0 };
    vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &format_count, nullptr);
    assert(format_count > 0);
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &format_count, details.formats.data());

    u32 present_mode_count{ 0 };
    vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, surface, &present_mode_count, nullptr);
    assert(present_mode_count > 0);
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, surface, &present_mode_count, details.present_modes.data());

    const auto [capabilities, formats, present_modes] = details;

    VkPresentModeKHR present_mode = present_modes[0];
    assert(!present_modes.empty());
    for (const auto& mode : present_modes)
    {
        if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            present_mode = mode;

        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            present_mode = mode;
    }

    assert(!formats.empty());
    for (const auto& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            surface_format = format;
    }

    i32 width{ 0 }, height{ 0 };
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actual_extent = {
        static_cast<u32>(width),
        static_cast<u32>(height)
    };

    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    surface_extent = actual_extent;

    u32 image_count{ capabilities.minImageCount };
    if (capabilities.minImageCount <= 2)
        image_count = 2;

    VkSwapchainCreateInfoKHR c_info{};
    c_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    c_info.surface = surface;
    c_info.minImageCount = image_count;
    c_info.imageFormat = surface_format.format;
    c_info.imageColorSpace = surface_format.colorSpace;
    c_info.imageExtent = surface_extent;
    c_info.imageArrayLayers = 1;
    c_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const auto [graphics_family, present_family] = queue_family_indices;
    const u32 indices[] = { graphics_family.value(), present_family.value() };
    if (graphics_family.value() != present_family.value())
    {
        c_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        c_info.queueFamilyIndexCount = 2;
        c_info.pQueueFamilyIndices = indices;
    }
    else
    {
        c_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        c_info.queueFamilyIndexCount = 0;
        c_info.pQueueFamilyIndices = nullptr;
    }
    c_info.preTransform = capabilities.currentTransform;
    c_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    c_info.presentMode = present_mode;
    c_info.clipped = VK_TRUE;
    c_info.oldSwapchain = old_swap_chain;

    if (vkCreateSwapchainKHR(device, &c_info, nullptr, &swap_chain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain!");

    if (old_swap_chain != nullptr)
    {
        for (const auto image_view : surface_image_views)
            vkDestroyImageView(device, image_view, nullptr);

        vkDestroySwapchainKHR(device, old_swap_chain, nullptr);
    }

    vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
    surface_images.resize(image_count);
    vkGetSwapchainImagesKHR(device, swap_chain, &image_count, surface_images.data());

    surface_image_views.resize(surface_images.size());
    for (usize i{ 0 }; i < surface_images.size(); ++i)
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = surface_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = surface_format.format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &create_info, nullptr, &surface_image_views[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain image views");
    }
}
}
