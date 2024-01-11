#pragma once
#include "vk_context.h"
#include "resources/vk_resource_manager.h"

namespace mas::gfx::vulkan
{
class Renderer final : public gfx::Renderer
{
public:
    explicit Renderer(GLFWwindow* window);
    ~Renderer() override = default;
    DISABLE_COPY_AND_MOVE(Renderer)

    void add_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data) override;

    void render(flecs::world* w) override;

private:
    std::shared_ptr<Context> context;
    flecs::world* world{ nullptr };
    ResourceManager resource_manager;
    Command draw_command;
};
}
