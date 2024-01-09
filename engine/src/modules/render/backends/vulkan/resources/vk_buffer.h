#pragma once
#include "../vk_context.h"

namespace mas::gfx::vulkan
{
class Buffer
{
public:
    Buffer() = delete;
    ~Buffer();
    DISABLE_COPY(Buffer)
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;
    Buffer(std::shared_ptr<Context> c, VkDeviceSize size, VkBufferUsageFlags usage_flags, VmaAllocationCreateFlags allocation_flags);
    Buffer(std::shared_ptr<Context> c, VkDeviceSize size, VkBufferUsageFlags usage_flags, VmaAllocationCreateFlags allocation_flags, const void* data);

    void copy_from(const Buffer& src);
private:
    std::shared_ptr<Context> context{ nullptr };
    VkBuffer buffer{ nullptr };
    VmaAllocation allocation{ nullptr };
    VmaAllocationInfo allocation_info;
};
}