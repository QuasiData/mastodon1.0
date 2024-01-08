#include "vk_renderer.h"

namespace mas::gfx::vulkan
{
Renderer::Renderer(GLFWwindow* window) : context(Context(window))
{

}

Renderer::~Renderer()
{
    
}

void Renderer::render(flecs::world* w)
{
    world = w;
}

void Renderer::add_mesh(std::vector<VertexP3N3U2T4>& vertices, std::vector<VertexIndexType>& indices)
{}
}
