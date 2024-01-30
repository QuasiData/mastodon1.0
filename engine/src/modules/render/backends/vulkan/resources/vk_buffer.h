#pragma once
#include "../vk_context.h"

#include <string>

namespace mas::gfx::vulkan
{
DEFINE_TYPED_ID(Buffer)

class Buffer
{
public:
    Buffer() = delete;
    ~Buffer();
    DISABLE_COPY(Buffer)
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;
    Buffer(std::shared_ptr<Context> c, VkDeviceSize size, VkBufferUsageFlags usage_flags, VmaAllocationCreateFlags allocation_flags = 0);
    Buffer(std::shared_ptr<Context> c, VkDeviceSize size, VkBufferUsageFlags usage_flags, const void* data, VmaAllocationCreateFlags allocation_flags = 0);

    void copy_from(const Buffer& src);

    void set_debug_name(const std::string& name);

private:
    std::shared_ptr<Context> context{ nullptr };

public:
    VkBuffer buffer{ nullptr };
    VmaAllocation allocation{ nullptr };
    VmaAllocationInfo allocation_info;
};
}