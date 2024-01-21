#include "vk_renderer.h"

#include "imgui/imgui.h"

#include <stdexcept>

namespace mas::gfx::vulkan
{
Renderer::Renderer(GLFWwindow* window)
    : context(std::make_shared<Context>(window)),
    resource_manager(context),
    ui_overlay(context),
    draw_command(Command(context, context->graphics_queue, context->queue_family_indices.graphics_family.value(), back_buffer_count))
{
    create_render_sync_objects();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(context->device);

    for (usize i{ 0 }; i < back_buffer_count; ++i)
    {
        vkDestroySemaphore(context->device, render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(context->device, image_available_semaphores[i], nullptr);
        vkDestroyFence(context->device, in_flight_fences[i], nullptr);
    }
}

void Renderer::add_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data)
{
    resource_manager.upload_models(model_data);
}

void Renderer::create_render_sync_objects()
{
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (usize i{ 0 }; i < back_buffer_count; ++i)
    {
        if (vkCreateSemaphore(context->device, &semaphore_create_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->device, &semaphore_create_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->device, &fence_create_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create a sync primitive!");
    }
}

void Renderer::render(flecs::world* w)
{
    UiOverlay::new_frame();

    ImGui::ShowDemoWindow();

    UiOverlay::end_frame();

    const auto im = image_available_semaphores[current_frame];
    const auto re = render_finished_semaphores[current_frame];
    const auto fr = in_flight_fences[current_frame];

    vkWaitForFences(context->device, 1, &fr, VK_TRUE, std::numeric_limits<u64>::max());

    u32 image_index{ 0 };
    const auto result = vkAcquireNextImageKHR(
        context->device, context->swap_chain,
        100, image_available_semaphores[current_frame],
        VK_NULL_HANDLE, &image_index);

    if (result != VK_SUCCESS)
    {
        vkDeviceWaitIdle(context->device);
        context->resize_swapchain();

        return;
    }

    vkResetFences(context->device, 1, &fr);

    const auto cmd = draw_command.begin(current_frame);
    const auto output_image = context->surface_images[image_index];
    {
        VkImageMemoryBarrier2 image_barrier{};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        image_barrier.image = output_image;
        image_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        image_barrier.srcAccessMask = 0;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkDependencyInfo dep_info{};
        dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep_info.pImageMemoryBarriers = &image_barrier;
        dep_info.imageMemoryBarrierCount = 1;

        vkCmdPipelineBarrier2(cmd, &dep_info);
    }

    ui_overlay.draw_cmd(cmd, context->surface_image_views[image_index]);

    {
        VkImageMemoryBarrier2 image_barrier{};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        image_barrier.image = output_image;
        image_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        image_barrier.dstAccessMask = 0;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkDependencyInfo dep_info{};
        dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep_info.pImageMemoryBarriers = &image_barrier;
        dep_info.imageMemoryBarrierCount = 1;

        vkCmdPipelineBarrier2(cmd, &dep_info);
    }
    draw_command.end(current_frame);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore wait_semaphores[] = { im };
    constexpr VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;

    const VkSemaphore signal_semaphores[] = { re };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(context->graphics_queue, 1, &submit_info, fr) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit command buffer to queue!");

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    const VkSwapchainKHR swap_chains[] = { context->swap_chain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    vkQueuePresentKHR(context->present_queue, &present_info);

    current_frame = (current_frame++) % back_buffer_count;
}
}
