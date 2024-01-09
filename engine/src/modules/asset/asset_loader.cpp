#include "asset_loader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO

#pragma warning( push )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4267 )
#include "taskflow/taskflow.hpp"
#include "spdlog/spdlog.h"
#include "tiny_gltf/tiny_gltf.h"
#pragma warning( pop )

namespace mas
{
namespace
{
void load(std::string path, const bool binary, gfx::MeshData& mesh_data, gfx::MaterialData& material_data)
{
    tinygltf::Model tiny_model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    if (binary)
    {
        if (const bool result = loader.LoadBinaryFromFile(&tiny_model, &err, &warn, path); !result)
        {
            spdlog::error("Failed to load gltf file: {}", path);
            throw std::runtime_error("Failed to load gltf file");
        }
    }
    else
    {
        if (const bool result = loader.LoadASCIIFromFile(&tiny_model, &err, &warn, path); !result)
        {
            spdlog::error("Failed to load gltf file: {}", path);
            throw std::runtime_error("Failed to load gltf file");
        }
    }

    if (tiny_model.scenes.size() > 1)
    {
        spdlog::error("More than one scene is not supported");
        throw std::runtime_error("More than once scene is not supported");
    }
}
}

AssetLoader::AssetLoader(AssetLoader&& other) noexcept
{
    *this = std::move(other);
}

AssetLoader& AssetLoader::operator=(AssetLoader&& other) noexcept
{
    this->renderer = other.renderer;
    other.renderer = nullptr;

    this->ascii_models_to_load = std::move(other.ascii_models_to_load);
    this->binary_models_to_load = std::move(other.binary_models_to_load);
    this->model_data = std::move(other.model_data);
    this->models = std::move(other.models);

    this->startup = other.startup;
    this->model_count = other.model_count;
    other.model_count = 0;

    this->string_hasher = other.string_hasher;

    return *this;
}

Model AssetLoader::load_gltf(const std::string& path)
{
    if (!startup)
    {
        spdlog::error("Attempted to load model after startup phase");
        throw std::runtime_error("Attempted to load model after startup phase");
    }

    const usize val = string_hasher(path);
    if (models.contains(val))
    {
        return models.at(val);
    }

    const MeshId mesh_id{ model_count };
    const MaterialId material_id{ model_count };
    const Model model{ mesh_id, material_id };
    models.insert({ val, model });
    model_count++;

    ascii_models_to_load.emplace_back(path, model);

    return model;
}

Model AssetLoader::loaf_glb(const std::string& path)
{
    if (!startup)
    {
        spdlog::error("Attempted to load model after startup phase");
        throw std::runtime_error("Attempted to load model after startup phase");
    }

    const usize val = string_hasher(path);
    if (models.contains(val))
    {
        return models.at(val);
    }

    const MeshId mesh_id{ model_count };
    const MaterialId material_id{ model_count };
    const Model model{ mesh_id, material_id };
    models.insert({ val, model });
    model_count++;

    binary_models_to_load.emplace_back(path, model);

    return model;
}

void AssetLoader::upload_all()
{
    tf::Executor executor;
    tf::Taskflow taskflow;

    for (usize i{ 0 }; i < ascii_models_to_load.size(); ++i)
    {
        taskflow.emplace(
            [this, i]()
            {
                gfx::MeshData mesh{};
                gfx::MaterialData material{};
                load(ascii_models_to_load[i].first, false, mesh, material);
                std::lock_guard<std::mutex> lock(mutex);
                model_data.emplace_back(ascii_models_to_load[i].second, mesh, material);
            });
    }

    for (usize i{ 0 }; i < binary_models_to_load.size(); ++i)
    {
        taskflow.emplace(
            [this, i]()
            {
                gfx::MeshData mesh{};
                gfx::MaterialData material{};
                load(binary_models_to_load[i].first, true, mesh, material);
                std::lock_guard<std::mutex> lock(mutex);
                model_data.emplace_back(binary_models_to_load[i].second, mesh, material);
            });
    }

    const auto start_time = std::chrono::high_resolution_clock::now();
    spdlog::info("Loading all models from disk");
    executor.run(taskflow).wait();
    const auto end_time = std::chrono::high_resolution_clock::now();
    spdlog::info("Done in {}s", std::chrono::duration<f32>(end_time - start_time).count());
    renderer->add_models(std::move(model_data));
}

void AssetLoader::inject_renderer(Renderer r)
{
    renderer = std::move(r);
}
}

