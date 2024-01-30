#include "vk_resource_manager.h"

#include "spdlog/spdlog.h"

#include <ranges>
#include <stdexcept>

namespace mas::gfx::vulkan
{
ResourceManager::ResourceManager(std::shared_ptr<Context> c)
    : context(std::move(c)), command(Command(context, context->graphics_queue, context->queue_family_indices.graphics_family.value(), 1))
{}

std::expected<BufferId, ResourceError> ResourceManager::add_buffer(Buffer buffer, const std::string& name)
{
    const auto id = BufferId{ next_buffer_id++ };

    if (!name.empty())
    {
        if (const auto result = named_buffers.insert({ name, id }).second; !result)
        {
            return std::unexpected(ResourceError::AlreadyExists);
        }
    }

    buffer_map.insert({ id, std::move(buffer) });

    return id;
}

std::expected<TextureId, ResourceError> ResourceManager::add_texture(Texture texture, const std::string& name)
{
    const auto id = TextureId{ next_text_id++ };

    if (!name.empty())
    {
        if (const auto result = named_textures.insert({ name, id }).second; !result)
        {
            return std::unexpected(ResourceError::AlreadyExists);
        }
    }

    texture_map.insert({ id, std::move(texture) });

    return id;
}

std::expected<void, ResourceError> ResourceManager::remove_buffer(const BufferId id)
{
    if (buffer_map.contains(id))
    {
        buffer_map.erase(id);
        return {};
    }

    return std::unexpected(ResourceError::NotFound);
}

std::expected<void, ResourceError> ResourceManager::remove_texture(const TextureId id)
{
    if (texture_map.contains(id))
    {
        texture_map.erase(id);
        return {};
    }

    return std::unexpected(ResourceError::NotFound);
}

std::optional<std::reference_wrapper<Buffer>> ResourceManager::get_buffer(const BufferId id)
{
    if (buffer_map.contains(id))
    {
        return std::reference_wrapper(buffer_map.at(id));
    }

    return std::nullopt;
}

std::optional<std::reference_wrapper<Buffer>> ResourceManager::get_buffer_by_name(const std::string& name)
{
    if (named_buffers.contains(name))
    {
        const auto id = named_buffers[name];

        assert(buffer_map.contains(id));

        return std::reference_wrapper(buffer_map.at(id));
    }

    return std::nullopt;
}

std::optional<BufferId> ResourceManager::get_buffer_id(const std::string& name)
{
    if (named_buffers.contains(name))
    {
        return named_buffers[name];
    }

    return std::nullopt;
}

std::optional<std::reference_wrapper<Texture>> ResourceManager::get_texture(const TextureId id)
{
    if (texture_map.contains(id))
    {
        return std::reference_wrapper(texture_map.at(id));
    }

    return std::nullopt;
}

std::optional<std::reference_wrapper<Texture>> ResourceManager::get_texture_by_name(const std::string& name)
{
    if (named_textures.contains(name))
    {
        const auto id = named_textures[name];

        assert(texture_map.contains(id));

        return std::reference_wrapper(texture_map.at(id));
    }

    return std::nullopt;
}

std::optional<TextureId> ResourceManager::get_texture_id(const std::string& name)
{
    if (named_textures.contains(name))
    {
        return named_textures[name];
    }

    return std::nullopt;
}

void ResourceManager::upload_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data)
{
    for (usize i{ 0 }; i < model_data.size(); ++i)
    {
        const auto [mesh_id, material_id] = std::get<Model>(model_data[i]);
        const auto& [vertices, indices] = std::get<gfx::MeshData>(model_data[i]);
        const auto& [present, albedo, normals, metallic_roughness, emissive] = std::get<gfx::MaterialData>(model_data[i]);

        Buffer vertex_buffer(context, vertices.size() * sizeof(VertexP3N3U2T4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.data());
        Buffer index_buffer(context, indices.size() * sizeof(VertexIndexType), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices.data());

        MaterialEntry material_entry{};

        if ((present & MaterialFlag::Albedo) != MaterialFlag::None)
        {
            material_entry.albedo = upload_texture(VK_FORMAT_R8G8B8A8_SRGB, albedo);
        }
        if ((present & MaterialFlag::Normal) != MaterialFlag::None)
        {
            material_entry.normal = upload_texture(VK_FORMAT_R8G8B8A8_UNORM, normals);
        }
        if ((present & MaterialFlag::MetallicRoughness) != MaterialFlag::None)
        {
            material_entry.metallic_roughness = upload_texture(VK_FORMAT_R8G8B8A8_UNORM, metallic_roughness);
        }
        if ((present & MaterialFlag::Emissive) != MaterialFlag::None)
        {
            material_entry.emissive = upload_texture(VK_FORMAT_R8G8B8A8_UNORM, emissive);
        }

        const auto vertex_buffer_id = BufferId{ next_buffer_id++ };
        const auto index_buffer_id = BufferId{ next_buffer_id++ };
        buffer_map.insert({ vertex_buffer_id , std::move(vertex_buffer) });
        buffer_map.insert({ index_buffer_id , std::move(index_buffer) });

        MeshEntry mesh_entry{ vertex_buffer_id, index_buffer_id };

        mesh_registry.insert({ mesh_id, mesh_entry });
        material_registry.insert({ material_id, material_entry });
    }
    vkDeviceWaitIdle(context->device);
}

TextureId ResourceManager::upload_texture(const VkFormat format, const TextureData& data)
{
    const Buffer buff(context, data.width * data.height * data.channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, data.data.data());

    Texture text(context, VK_IMAGE_TYPE_2D, format, VK_IMAGE_ASPECT_COLOR_BIT,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                 0, static_cast<u32>(data.width), static_cast<u32>(data.height), 1, 1, 1);

    text.create_sampler();

    VkBufferImageCopy image_copy{};
    image_copy.imageSubresource = VkImageSubresourceLayers{ text.aspect, 0, 0, 1 };
    image_copy.imageExtent = text.extent;
    image_copy.imageOffset = { 0, 0, 0 };
    image_copy.bufferOffset = 0;

    copy_buffer_to_texture(buff, text, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { image_copy });

    const auto id = TextureId{ next_text_id++ };
    texture_map.insert({ id, std::move(text) });

    return id;
}

void ResourceManager::copy_buffer_to_texture(const Buffer& buffer, Texture& texture, const VkImageLayout new_layout, const std::vector<VkBufferImageCopy>& regions) const
{
    const auto cmd = command.begin();
    texture.set_layout_cmd(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    vkCmdCopyBufferToImage(cmd, buffer.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<u32>(regions.size()), regions.data());
    texture.set_layout_cmd(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
    command.flush();
}
}
