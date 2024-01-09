#include "vk_renderer.h"

#include "resources/vk_buffer.h"

namespace mas::gfx::vulkan
{
Renderer::Renderer(GLFWwindow* window)
: context(std::make_shared<Context>(window))
{

}

Renderer::~Renderer()
{
    context->destroy();
}

void Renderer::render(flecs::world* w)
{
    world = w;
}

void Renderer::add_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>> model_data)
{
    const auto& data = std::get<MeshData>(model_data[0]);

    Buffer(context, data.vertices.size() * sizeof(VertexP3N3U2T4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, data.vertices.data());
}
}
