#include "vk_renderer.h"

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

}
}
