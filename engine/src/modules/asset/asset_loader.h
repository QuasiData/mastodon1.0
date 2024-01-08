#pragma once
#include "common.h"
#include "modules/render/render_module.h"

#include <unordered_map>
#include <string>

namespace mas
{
DEFINE_TYPED_ID(Mesh)
DEFINE_TYPED_ID(Material)

struct Model
{
    MeshId mesh_id;
    MaterialId material_id;
};

class AssetLoader
{
public:
    AssetLoader() = default;
    ~AssetLoader() = default;
    DISABLE_COPY_AND_MOVE(AssetLoader)

private:
    std::vector<std::string> models_to_load{};
    std::vector<gfx::MeshData> mesh_data{};
};
}