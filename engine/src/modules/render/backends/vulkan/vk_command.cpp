#include "vk_command.h"

#include <cassert>
#include <stdexcept>

namespace mas::gfx::vulkan
{
Command::~Command()
{
    vkDestroyCommandPool(context->device, command_pool, nullptr);
}

Command::Command(Command&& other) noexcept
{
    *this = std::move(other);
}

Command& Command::operator=(Command&& other) noexcept
{
    this->context = std::move(other.context);
    this->queue = other.queue;
    this->command_pool = other.command_pool;
    this->command_buffers = std::move(other.command_buffers);

    other.context = nullptr;
    other.queue = nullptr;
    other.command_pool = nullptr;
    other.command_buffers.clear();

    return *this;
}

Command::Command(std::shared_ptr<Context> c, const VkQueue q, const u32 queue_family_index, const u32 buffer_count)
{
    context = std::move(c);
    queue = q;

    VkCommandPoolCreateInfo command_pool_ci{};
    command_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.queueFamilyIndex = queue_family_index;

    if (vkCreateCommandPool(context->device, &command_pool_ci, nullptr, &command_pool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");

    command_buffers.resize(buffer_count);

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = buffer_count;

    if (vkAllocateCommandBuffers(context->device, &allocate_info, command_buffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command buffer");
}

VkCommandBuffer Command::begin(const u32 index) const
{
    vkResetCommandBuffer(command_buffers[index], 0);
    assert(index < command_buffers.size());

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffers[index], &begin_info);

    return command_buffers[index];
}

void Command::end(const u32 index) const
{
    assert(index < command_buffers.size());

    vkEndCommandBuffer(command_buffers[index]);
}


void Command::flush(const u32 index) const
{
    vkEndCommandBuffer(command_buffers[index]);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pCommandBuffers = &command_buffers[index];
    submit_info.commandBufferCount = 1;

    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkResetCommandBuffer(command_buffers[index], 0);
}
}
