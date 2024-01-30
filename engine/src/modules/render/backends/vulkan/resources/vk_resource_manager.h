#pragma once
#include "../vk_context.h"
#include "../vk_command.h"
#include "vk_buffer.h"
#include "vk_texture.h"

#include <unordered_map>
#include <string>
#include <expected>

namespace mas::gfx::vulkan
{
enum class ResourceError
{
    AlreadyExists,
    NotFound,
};

struct MeshEntry
{
    BufferId vertex_buffer{ id::invalid_id };
    BufferId index_buffer{ id::invalid_id };
};

struct MaterialEntry
{
    TextureId albedo{ id::invalid_id };
    TextureId normal{ id::invalid_id };
    TextureId metallic_roughness{ id::invalid_id };
    TextureId emissive{ id::invalid_id };
};

class ResourceManager
{
public:
    ResourceManager() = delete;
    ~ResourceManager() = default;
    DISABLE_COPY_AND_MOVE(ResourceManager)
    explicit ResourceManager(std::shared_ptr<Context> c);

    [[nodiscard]] std::expected<BufferId, ResourceError> add_buffer(Buffer buffer, const std::string& name = "");
    [[nodiscard]] std::expected<TextureId, ResourceError> add_texture(Texture texture, const std::string& name = "");

    [[nodiscard]] std::expected<void, ResourceError> remove_buffer(BufferId id);
    [[nodiscard]] std::expected<void, ResourceError> remove_texture(TextureId id);

    [[nodiscard]] std::optional<std::reference_wrapper<Buffer>> get_buffer(BufferId id);
    [[nodiscard]] std::optional<std::reference_wrapper<Buffer>> get_buffer_by_name(const std::string& name);
    [[nodiscard]] std::optional<BufferId> get_buffer_id(const std::string& name);

    [[nodiscard]] std::optional<std::reference_wrapper<Texture>> get_texture(TextureId id);
    [[nodiscard]] std::optional<std::reference_wrapper<Texture>> get_texture_by_name(const std::string& name);
    [[nodiscard]] std::optional<TextureId> get_texture_id(const std::string& name);

    void upload_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data);

private:
    [[nodiscard]] TextureId upload_texture(const VkFormat format, const TextureData& data);

    void copy_buffer_to_texture(const Buffer& buffer, Texture& texture, VkImageLayout new_layout, const std::vector<VkBufferImageCopy>& regions) const;

    std::shared_ptr<Context> context{ nullptr };
    Command command;

    id::IdType next_buffer_id{ 0 };
    id::IdType next_text_id{ 0 };

    std::unordered_map<std::string, BufferId> named_buffers{};
    std::unordered_map<std::string, TextureId> named_textures{};

    std::unordered_map<id::IdType, MeshEntry> mesh_registry{};
    std::unordered_map<id::IdType, MaterialEntry> material_registry{};

    std::unordered_map<id::IdType, Buffer> buffer_map{};
    std::unordered_map<id::IdType, Texture> texture_map{};
};
}