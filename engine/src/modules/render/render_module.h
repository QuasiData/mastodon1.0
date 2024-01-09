#pragma once
#include "common.h"

#include "flecs/flecs.h"
#include "glm/glm.hpp"

#include <memory>
#include <vector>

namespace mas
{
DEFINE_TYPED_ID(Mesh)
DEFINE_TYPED_ID(Material)

struct Model
{
    MeshId mesh_id;
    MaterialId material_id;
};

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

struct MeshData
{
    std::vector<VertexP3N3U2T4> vertices{};
    std::vector<VertexIndexType> indices{};
};

struct TextureData
{
    u32 width{ 0 };
    u32 height{ 0 };
    u32 channels{ 0 };
    std::vector<u8> data{};
};

enum class MaterialFlag
{
    None = 0b0000,
    Albedo = 0b0001,
    Normal = 0b0010,
    MetallicRoughness = 0b0100,
    Emissive = 0b1000,

};

struct MaterialData
{
    MaterialFlag present{ MaterialFlag::None };
    TextureData albedo{};
    TextureData normals{};
    TextureData metallic_roughness{};
    TextureData emissive{};
};

class Renderer
{
public:
    Renderer() = default;
    virtual ~Renderer() = default;
    DISABLE_COPY_AND_MOVE(Renderer)

    virtual void render(flecs::world* world) = 0;

    virtual void add_models(std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>> model_data) = 0;
};
}

using Renderer = std::shared_ptr<gfx::Renderer>;

struct RenderModule
{
    // ReSharper disable once CppNonExplicitConvertingConstructor
    RenderModule(flecs::world& world);
};
}
