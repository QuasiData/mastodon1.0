// ReSharper disable CppClangTidyClangDiagnosticSwitchEnum
#include "asset_loader.h"
#include "tangents.h"

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
enum class ComponentType : u32
{
    SignedByte = 5120,
    UnsignedByte = 5121,
    SignedShort = 5122,
    UnsignedShort = 5123,
    UnsignedInt = 5125,
    Float = 5126,
};

enum class StorageType : u32
{
    Scalar = 64 + 1,
    Vec2 = 2,
    Vec3 = 3,
    Vec4 = 4,
    Mat2 = 32 + 2,
    Mat3 = 32 + 3,
    Mat4 = 32 + 4,
    Vector = 64 + 4,
    Matrix = 64 + 16,
};

void load(std::string path, const bool binary, gfx::MeshData& mesh_data, gfx::MaterialData& material_data)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    if (binary)
    {
        if (const bool result = loader.LoadBinaryFromFile(&model, &err, &warn, path); !result)
        {
            spdlog::error("Failed to load gltf file: {}", path);
            throw std::runtime_error("Failed to load gltf file");
        }
    }
    else
    {
        if (const bool result = loader.LoadASCIIFromFile(&model, &err, &warn, path); !result)
        {
            spdlog::error("Failed to load gltf file: {}", path);
            throw std::runtime_error("Failed to load gltf file");
        }
    }

    gfx::TextureData albedo_texture_data{};
    gfx::TextureData normal_texture_data{};
    gfx::TextureData metallic_roughness_texture_data{};
    gfx::TextureData emissive_texture_data{};
    std::vector<glm::vec3> position_vec{};
    std::vector<glm::vec3> normals_vec{};
    std::vector<glm::vec2> uv_vec{};

    std::vector<gfx::VertexP3N3U2T4> vertex_vec{};
    std::vector<gfx::VertexIndexType> index_vec{};

    if (model.scenes.size() > 1)
    {
        spdlog::error("More than one scene is not supported");
        throw std::runtime_error("More than once scene is not supported");
    }

    if (model.nodes.size() > 1)
    {
        spdlog::error("More than one node is not supported");
        throw std::runtime_error("More than one node is not supported");
    }

    for (const auto& node : model.nodes)
    {
        if (!node.children.empty())
        {
            spdlog::error("Nodes with children are not supported");
            throw std::runtime_error("Nodes with children are not supported");
        }

        const auto mesh = model.meshes[node.mesh];
        const auto& prims = mesh.primitives;

        if (prims.size() > 1)
        {
            spdlog::error("Meshes with multiple primitives are not supported");
            throw std::runtime_error("Meshes with multiple primitives are not supported");
        }

        for (const auto& prim : prims)
        {
            const i32 pos_index = prim.attributes.at("POSITION");
            const i32 norm_index = prim.attributes.at("NORMAL");
            const i32 uv_index = prim.attributes.at("TEXCOORD_0");
            const i32 indices_index = prim.indices;

            const tinygltf::Accessor& pos_acc = model.accessors[pos_index];
            const tinygltf::Accessor& norm_acc = model.accessors[norm_index];
            const tinygltf::Accessor& uv_acc = model.accessors[uv_index];
            const tinygltf::Accessor& indices_acc = model.accessors[indices_index];

            const ComponentType pos_comp_type = static_cast<ComponentType>(pos_acc.componentType);
            const ComponentType norm_comp_type = static_cast<ComponentType>(norm_acc.componentType);
            const ComponentType uv_comp_type = static_cast<ComponentType>(uv_acc.componentType);
            const ComponentType indices_comp_type = static_cast<ComponentType>(indices_acc.componentType);

            const StorageType pos_store_type = static_cast<StorageType>(pos_acc.type);
            const StorageType norm_store_type = static_cast<StorageType>(norm_acc.type);
            const StorageType uv_store_type = static_cast<StorageType>(uv_acc.type);
            const StorageType indices_store_type = static_cast<StorageType>(indices_acc.type);

            const tinygltf::BufferView& pos_buff_view = model.bufferViews[pos_acc.bufferView];
            const tinygltf::BufferView& norm_buff_view = model.bufferViews[norm_acc.bufferView];
            const tinygltf::BufferView& uv_buff_view = model.bufferViews[uv_acc.bufferView];
            const tinygltf::BufferView& indices_buff_view = model.bufferViews[indices_acc.bufferView];

            u8* pos_buffer = model.buffers[pos_buff_view.buffer].data.data() + pos_buff_view.byteOffset;
            u8* norm_buffer = model.buffers[norm_buff_view.buffer].data.data() + norm_buff_view.byteOffset;
            u8* uv_buffer = model.buffers[uv_buff_view.buffer].data.data() + uv_buff_view.byteOffset;
            u8* indices_buffer = model.buffers[indices_buff_view.buffer].data.data() + indices_buff_view.byteOffset;

            switch (pos_store_type)
            {
                case StorageType::Vec3:
                    switch (pos_comp_type)
                    {
                        case ComponentType::Float:
                            for (usize i{ 0 }; i < pos_buff_view.byteLength / sizeof(glm::vec3); ++i)
                            {
                                position_vec.emplace_back(*reinterpret_cast<glm::vec3*>(pos_buffer));
                                pos_buffer += sizeof(glm::vec3);
                            }
                            break;
                        default:
                            spdlog::error("Not supported position component type");
                            throw std::runtime_error("Not supported position component type");
                    }
                    break;
                default:
                    spdlog::error("Not supported position storage type");
                    throw std::runtime_error("Not supported position storage type");
            }

            switch (norm_store_type)
            {
                case StorageType::Vec3:
                    switch (norm_comp_type)
                    {
                        case ComponentType::Float:
                            for (usize i{ 0 }; i < norm_buff_view.byteLength / sizeof(glm::vec3); ++i)
                            {
                                normals_vec.emplace_back(*reinterpret_cast<glm::vec3*>(norm_buffer));
                                norm_buffer += sizeof(glm::vec3);
                            }
                            break;
                        default:
                            spdlog::error("Not supported normal component type");
                            throw std::runtime_error("Not supported normal component type");
                    }
                    break;
                default:
                    spdlog::error("Not supported normal storage type");
                    throw std::runtime_error("Not supported normal storage type");
            }

            switch (uv_store_type)
            {
                case StorageType::Vec2:
                    switch (uv_comp_type)
                    {
                        case ComponentType::Float:
                            for (usize i{ 0 }; i < uv_buff_view.byteLength / sizeof(glm::vec2); ++i)
                            {
                                uv_vec.emplace_back(*reinterpret_cast<glm::vec2*>(uv_buffer));
                                uv_buffer += sizeof(glm::vec2);
                            }
                            break;
                        default:
                            spdlog::error("Not supported uv component type");
                            throw std::runtime_error("Not supported uv component type");
                    }
                    break;
                default:
                    spdlog::error("Not supported uv storage type");
                    throw std::runtime_error("Not supported uv storage type");
            }

            switch (indices_store_type)
            {
                case StorageType::Scalar:
                    switch (indices_comp_type)
                    {
                        case ComponentType::SignedByte:
                            for (usize i{ 0 }; i < indices_buff_view.byteLength / sizeof(i8); ++i)
                            {
                                index_vec.emplace_back(*reinterpret_cast<i8*>(indices_buffer));
                                indices_buffer += sizeof(i8);
                            }
                            break;
                        case ComponentType::UnsignedByte:
                            for (usize i{ 0 }; i < indices_buff_view.byteLength / sizeof(u8); ++i)
                            {
                                index_vec.emplace_back(*indices_buffer);
                                indices_buffer += sizeof(u8);
                            }
                            break;
                        case ComponentType::SignedShort:
                            for (usize i{ 0 }; i < indices_buff_view.byteLength / sizeof(i16); ++i)
                            {
                                index_vec.emplace_back(*reinterpret_cast<i16*>(indices_buffer));
                                indices_buffer += sizeof(i16);
                            }
                            break;
                        case ComponentType::UnsignedShort:
                            for (usize i{ 0 }; i < indices_buff_view.byteLength / sizeof(u16); ++i)
                            {
                                index_vec.emplace_back(*reinterpret_cast<u16*>(indices_buffer));
                                indices_buffer += sizeof(u16);
                            }
                            break;
                        case ComponentType::UnsignedInt:
                            for (usize i{ 0 }; i < indices_buff_view.byteLength / sizeof(u32); ++i)
                            {
                                index_vec.emplace_back(*reinterpret_cast<u32*>(indices_buffer));
                                indices_buffer += sizeof(u32);
                            }
                            break;
                        default:
                            spdlog::error("Not supported index component type");
                            throw std::runtime_error("Not supported index component type");
                    }
                    break;
                default:
                    spdlog::error("Not supported index storage type");
                    throw std::runtime_error("Not supported index storage type");
            }

            auto mat = model.materials[prim.material];

            const auto albedo_index = mat.pbrMetallicRoughness.baseColorTexture.index;
            if (albedo_index != -1)
            {
                material_data.albedo.data = std::move(model.images[albedo_index].image);
                material_data.albedo.width = static_cast<usize>(model.images[albedo_index].width);
                material_data.albedo.height = static_cast<usize>(model.images[albedo_index].height);
                material_data.albedo.channels = 4;
                material_data.present |= gfx::MaterialFlag::Albedo;
            }
            else
            {
                material_data.present &= ~gfx::MaterialFlag::Albedo;
            }

            const auto normal_index = mat.normalTexture.index;
            if (normal_index != -1)
            {
                material_data.normals.data = std::move(model.images[normal_index].image);
                material_data.normals.width = static_cast<usize>(model.images[normal_index].width);
                material_data.normals.height = static_cast<usize>(model.images[normal_index].height);
                material_data.normals.channels = 4;
                material_data.present |= gfx::MaterialFlag::Normal;
            }
            else
            {
                material_data.present &= ~gfx::MaterialFlag::Normal;
            }

            const auto metallic_roughness_index = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
            if (metallic_roughness_index != -1)
            {
                material_data.metallic_roughness.data = std::move(model.images[metallic_roughness_index].image);
                material_data.metallic_roughness.width = static_cast<usize>(model.images[metallic_roughness_index].width);
                material_data.metallic_roughness.height = static_cast<usize>(model.images[metallic_roughness_index].height);
                material_data.metallic_roughness.channels = 4;
                material_data.present |= gfx::MaterialFlag::MetallicRoughness;
            }
            else
            {
                material_data.present &= ~gfx::MaterialFlag::MetallicRoughness;
            }

            const auto emissive_index = mat.emissiveTexture.index;
            if (emissive_index != -1)
            {
                material_data.emissive.data = std::move(model.images[emissive_index].image);
                material_data.emissive.width = static_cast<usize>(model.images[emissive_index].width);
                material_data.emissive.height = static_cast<usize>(model.images[emissive_index].height);
                material_data.emissive.channels = 4;
                material_data.present |= gfx::MaterialFlag::Emissive;
            }
            else
            {
                material_data.present &= ~gfx::MaterialFlag::Emissive;
            }
        }
    }

    vertex_vec.reserve(position_vec.size());

    for (usize i{ 0 }; i < position_vec.size(); ++i)
    {
        vertex_vec.emplace_back(position_vec[i], normals_vec[i], uv_vec[i]);
    }

    details::TangentCalculator calculator{};
    details::IntermediateMeshData intermediate_mesh_data{ std::move(vertex_vec), std::move(index_vec), path, details::FaceType::Triangles };
    calculator.calculate(&intermediate_mesh_data);

    mesh_data.vertices = std::move(intermediate_mesh_data.vertices);
    mesh_data.indices = std::move(intermediate_mesh_data.indices);
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

Model AssetLoader::load_glb(const std::string& path)
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
