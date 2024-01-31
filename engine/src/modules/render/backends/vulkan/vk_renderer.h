#pragma once
#include "vk_context.h"
#include "vk_ui.h"
#include "render_graph.h"
#include "resources/vk_resource_manager.h"

namespace mas::gfx::vulkan
{
class Renderer final : public gfx::Renderer
{
public:
    explicit Renderer(GLFWwindow* window, flecs::world* world);
    ~Renderer() override;
    DISABLE_COPY_AND_MOVE(Renderer)

    void add_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data) override;

    void render(flecs::world* w) override;

    void startup_done() override;

private:
    void create_render_sync_objects();

    std::shared_ptr<Context> context;
    flecs::world* world{ nullptr };
    ResourceManager resource_manager;
    RenderGraph render_graph;
    UiOverlay ui_overlay;
    Command draw_command;
    u32 current_frame{ 0 };

    // Sync objects
    std::vector<VkSemaphore> image_available_semaphores{ back_buffer_count };
    std::vector<VkSemaphore> render_finished_semaphores{ back_buffer_count };
    std::vector<VkFence> in_flight_fences{ back_buffer_count };
};
}
