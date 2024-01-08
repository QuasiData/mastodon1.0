#pragma once
#include "vk_context.h"
#include "modules/render/render_module.h"

namespace mas::gfx::vulkan
{
class Renderer final : public gfx::Renderer
{
public:
    explicit Renderer(GLFWwindow* window);
    ~Renderer() override;
    DISABLE_COPY_AND_MOVE(Renderer)

    void render(flecs::world* w) override;
    void add_mesh(std::vector<VertexP3N3U2T4>& vertices, std::vector<VertexIndexType>& indices) override;

private:
    Context context;
    flecs::world* world{ nullptr };
};
}