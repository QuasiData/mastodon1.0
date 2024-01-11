#pragma once
#include "vk_context.h"

namespace mas::gfx::vulkan
{
class UiOverlay
{
public:
    UiOverlay() = delete;
    ~UiOverlay();
    DISABLE_COPY_AND_MOVE(UiOverlay)
    explicit UiOverlay(std::shared_ptr<Context> c);

    static void new_frame();
    static void end_frame();
    void draw_cmd(VkCommandBuffer cmd, VkImageView target_view) const;
private:
    std::shared_ptr<Context> context{ nullptr };
    VkDescriptorPool imgui_pool{ nullptr };
};
}
