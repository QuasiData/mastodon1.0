#include "vk_resource_manager.h"

#include <stdexcept>

namespace mas::gfx::vulkan
{
ResourceManager::~ResourceManager()
{
    throw std::logic_error("Not implemented");
}

ResourceManager::ResourceManager(std::shared_ptr<Context> c)
    : context(std::move(c)), command(Command(context, context->graphics_queue, context->queue_family_indices.graphics_family.value(), 1))
{}

void ResourceManager::upload_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data)
{
    for (usize i{ 0 }; i < model_data.size(); ++i)
    {
        const auto model = std::get<Model>(model_data[i]);
        const auto& [vertices, indices] = std::get<gfx::MeshData>(model_data[i]);
        const auto& [present, albedo, normals, metallic_roughness, emissive] = std::get<gfx::MaterialData>(model_data[i]);

        MeshEntry mesh_entry{};

        Buffer vertex_buffer(context, vertices.size() * sizeof(VertexP3N3U2T4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, vertices.data());
        Buffer index_buffer(context, indices.size() * sizeof(VertexIndexType), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, indices.data());

        if ((present & MaterialFlag::Albedo) != MaterialFlag::None)
        {
            Buffer albedo_buff(context, albedo.width * albedo.height * albedo.channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, albedo.data.data());

            Texture albedo_text(context, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT,
                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                                0, static_cast<u32>(albedo.width), static_cast<u32>(albedo.height), 1, 1, 1);

            albedo_text.create_sampler();

            VkBufferImageCopy image_copy{};
            image_copy.imageSubresource = VkImageSubresourceLayers{ albedo_text.aspect, 0, 0, 1 };
            image_copy.imageExtent = albedo_text.extent;
            image_copy.imageOffset = { 0, 0, 0 };
            image_copy.bufferOffset = 0;

            copy_buffer_to_texture(albedo_buff, albedo_text, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { image_copy });

            texture_map.insert({ TextureId{ next_text_id++ }, std::move(albedo_text) });
        }
    }
}

void ResourceManager::copy_buffer_to_texture(const Buffer& buffer, Texture& texture, const VkImageLayout new_layout, const std::vector<VkBufferImageCopy>& regions) const
{
    const auto cmd = command.begin();
    texture.set_layout_cmd(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    vkCmdCopyBufferToImage(cmd, buffer.buffer, texture.image, new_layout, static_cast<u32>(regions.size()), regions.data());
    command.flush();
}
}
