#pragma once
#include "common.h"
#include "modules/render/render_module.h"

#include "volk/volk.h"
#include "vma/vk_mem_alloc.h"

#include <optional>
#include <vector>
#include <memory>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

namespace mas::gfx::vulkan
{
constexpr u32 back_buffer_count{ 2 };

#if _DEBUG
constexpr bool enable_validation{ true };
#else
constexpr bool enable_validation{ true };
#endif

struct QueueFamilyIndices
{
    std::optional<u32> graphics_family = std::nullopt;
    //std::optional<u32> compute_family = std::nullopt;
    //std::optional<u32> transfer_family = std::nullopt;
    std::optional<u32> present_family = std::nullopt;

    [[nodiscard]] bool has_graphics_queue() const { return graphics_family.has_value(); }
    //[[nodiscard]] bool has_compute_queue() const { return compute_family.has_value(); }
    //[[nodiscard]] bool has_transfer_queue() const { return transfer_family.has_value(); }
    [[nodiscard]] bool has_present_queue() const { return present_family.has_value(); }

    [[nodiscard]] bool is_complete() const { return (has_graphics_queue() && has_present_queue()); }
};

class Context
{
public:
    explicit Context(GLFWwindow* w);
    ~Context();
    DISABLE_COPY_AND_MOVE(Context)

    void resize_swapchain();

private:
    static VkInstance create_instance();

    void create_device();

    void finalize_swapchain();

public:
    GLFWwindow* window{ nullptr };
    VkInstance instance{ nullptr };
    VkSurfaceKHR surface{ nullptr };
    VkPhysicalDevice phys_device{ nullptr };
    VkDevice device{ nullptr };
    QueueFamilyIndices queue_family_indices{};
    VkQueue graphics_queue{ nullptr };
    VkQueue present_queue{ nullptr };
    VmaAllocator allocator{ nullptr };
    VkSwapchainKHR swap_chain{ nullptr };
    VkSurfaceFormatKHR surface_format{};
    VkExtent2D surface_extent{};
    std::vector<VkImage> surface_images{};
    std::vector<VkImageView> surface_image_views{};
};
}