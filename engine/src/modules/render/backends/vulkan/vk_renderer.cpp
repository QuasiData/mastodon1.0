#include "vk_renderer.h"

namespace mas::gfx::vulkan
{
Renderer::Renderer(GLFWwindow* window)
: context(Context(window))
{

}

Renderer::~Renderer()
{
    context.destroy();
}

void Renderer::render(flecs::world* w)
{
    world = w;
}

void Renderer::add_models(std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>> model_data)
{

}
}
