#pragma once
#include "common.h"

#include "flecs/flecs.h"
#include "glm/glm.hpp"

#include <memory>
#include <vector>

namespace mas
{
namespace gfx
{
struct Camera
{
    glm::mat4 view{ 0.0f };
    glm::mat4 proj{ 0.0f };
    f32 aspect{ 0.0f };
    f32 z_near{ 0.0f };
    f32 z_far{ 0.0f };
    f32 fov{ 0.0f };
};

using VertexIndexType = u32;

struct VertexP3N3U2T4
{
    glm::vec3 pos;
    glm::vec3 normal;
};

class Renderer
{
public:
    Renderer() = default;
    virtual ~Renderer() = default;
    DISABLE_COPY_AND_MOVE(Renderer)

    virtual void render(flecs::world* world) = 0;

    virtual void add_mesh(std::vector<VertexP3N3U2T4>& vertices, std::vector<VertexIndexType>& indices) = 0;
};
}

using Renderer = std::shared_ptr<gfx::Renderer>;

struct RenderModule
{
    // ReSharper disable once CppNonExplicitConvertingConstructor
    RenderModule(flecs::world& world);
};
}
