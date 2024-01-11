#include "vk_renderer.h"

namespace mas::gfx::vulkan
{
Renderer::Renderer(GLFWwindow* window)
    : context(std::make_shared<Context>(window)),
    resource_manager(context),
    draw_command(Command(context, context->graphics_queue, context->queue_family_indices.graphics_family.value(), back_buffer_count))
{}

void Renderer::add_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data)
{
    resource_manager.upload_models(model_data);
}

void Renderer::render(flecs::world* w)
{
    world = w;
}
}
