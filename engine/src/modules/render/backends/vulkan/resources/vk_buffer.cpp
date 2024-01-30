#include "vk_buffer.h"
#include "../vk_command.h"

#include "spdlog/spdlog.h"

#include <stdexcept>

namespace mas::gfx::vulkan
{
Buffer::~Buffer()
{
    if (buffer)
        vmaDestroyBuffer(context->allocator, buffer, allocation);
}

Buffer::Buffer(Buffer&& other) noexcept
{
    *this = std::move(other);
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    this->context = other.context;
    this->buffer = other.buffer;
    this->allocation = other.allocation;
    this->allocation_info = other.allocation_info;

    other.context = nullptr;
    other.buffer = nullptr;
    other.allocation = nullptr;
    other.allocation_info = VmaAllocationInfo{};

    return *this;
}

Buffer::Buffer(std::shared_ptr<Context> c, const VkDeviceSize size, const VkBufferUsageFlags usage_flags, const VmaAllocationCreateFlags allocation_flags)
{
    context = std::move(c);

    VkBufferCreateInfo buffer_ci{};
    buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_ci.usage = usage_flags;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.pQueueFamilyIndices = &context->queue_family_indices.graphics_family.value();
    buffer_ci.size = size;
    buffer_ci.pNext = nullptr;

    VmaAllocationCreateInfo alloc_ci{};
    alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_ci.flags = allocation_flags;

    if (vmaCreateBuffer(context->allocator, &buffer_ci, &alloc_ci, &buffer, &allocation, &allocation_info) != VK_SUCCESS)
    {
        spdlog::error("Failed to create buffer");
        throw std::runtime_error("Failed to create buffer");
    }
}

Buffer::Buffer(std::shared_ptr<Context> c, const VkDeviceSize size, const VkBufferUsageFlags usage_flags, const void* data, const VmaAllocationCreateFlags allocation_flags)
{
    context = std::move(c);

    const auto staging_buffer = Buffer(
        context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    memcpy(staging_buffer.allocation_info.pMappedData, data, size);

    auto temp_buffer = Buffer(
        context, size, usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, allocation_flags);

    temp_buffer.copy_from(staging_buffer);

    *this = std::move(temp_buffer);
}

void Buffer::copy_from(const Buffer& src)
{
    if (allocation_info.size != src.allocation_info.size)
    {
        spdlog::error("Attempted to copy from buffer with size: {0} to buffer with size: {1}", src.allocation_info.size, allocation_info.size);
        throw std::runtime_error("Mismatched size");
    }

    VkBufferCopy2 copy{};
    copy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = allocation_info.size;

    VkCopyBufferInfo2 copy_info{};
    copy_info.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
    copy_info.dstBuffer = buffer;
    copy_info.srcBuffer = src.buffer;
    copy_info.pRegions = &copy;
    copy_info.regionCount = 1;

    const Command command{ context, context->graphics_queue, context->queue_family_indices.graphics_family.value() };
    const auto cmd = command.begin();
    vkCmdCopyBuffer2(cmd, &copy_info);
    command.flush();
}

void Buffer::set_debug_name(const std::string& name)
{
    if constexpr (enable_validation)
    {
        auto name_info = VkDebugUtilsObjectNameInfoEXT{};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        name_info.objectHandle = reinterpret_cast<u64>(buffer);
        name_info.objectType = VK_OBJECT_TYPE_BUFFER;
        name_info.pObjectName = name.c_str();

        vkSetDebugUtilsObjectNameEXT(context->device, &name_info);
    }
}
}
