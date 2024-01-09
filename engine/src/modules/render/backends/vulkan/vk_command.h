#pragma once
#include "vk_context.h"

namespace mas::gfx::vulkan
{
class Command
{
public:
    Command() = delete;
    ~Command();
    DISABLE_COPY(Command)
    Command(Command&& other) noexcept;
    Command& operator=(Command&& other) noexcept;
    Command(std::shared_ptr<Context> c, VkQueue q, const u32 queue_family_index, const u32 buffer_count = 1);

    [[nodiscard]] VkCommandBuffer begin(const u32 index = 0) const;

    void end(const u32 index = 0) const;

    void flush(const u32 index = 0) const;
private:
    std::shared_ptr<Context> context{ nullptr };
    VkQueue queue{ nullptr };
    VkCommandPool command_pool{ nullptr };
    std::vector<VkCommandBuffer> command_buffers{};
};
}