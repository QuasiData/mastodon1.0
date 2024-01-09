#pragma once
#include "common.h"
#include "modules/render/render_module.h"

#include <unordered_map>
#include <string>
#include <mutex>

namespace mas
{
class App;

class AssetLoader
{
    friend class App;
public:
    AssetLoader() = default;
    ~AssetLoader() = default;
    AssetLoader(AssetLoader&&) noexcept;
    AssetLoader& operator=(AssetLoader&&) noexcept;
    DISABLE_COPY(AssetLoader)

    Model load_gltf(const std::string& path);

    Model load_glb(const std::string& path);

private:
    void upload_all();

    void inject_renderer(Renderer r);

    Renderer renderer{ nullptr };
    bool startup{ true };
    usize model_count{ 0 };
    std::hash<std::string> string_hasher{};
    std::unordered_map<usize, Model> models{};
    std::vector<std::pair<std::string, Model>> ascii_models_to_load{};
    std::vector<std::pair<std::string, Model>> binary_models_to_load{};
    std::mutex mutex{};
    std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>> model_data{};
};
}