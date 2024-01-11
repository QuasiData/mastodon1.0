#include "vk_resource_manager.h"

#include <ranges>
#include <stdexcept>

namespace mas::gfx::vulkan
{
ResourceManager::ResourceManager(std::shared_ptr<Context> c)
    : context(std::move(c)), command(Command(context, context->graphics_queue, context->queue_family_indices.graphics_family.value(), 1))
{}

void ResourceManager::upload_models(const std::vector<std::tuple<Model, gfx::MeshData, gfx::MaterialData>>& model_data)
{
    for (usize i{ 0 }; i < model_data.size(); ++i)
    {
        const auto [mesh_id, material_id] = std::get<Model>(model_data[i]);
        const auto& [vertices, indices] = std::get<gfx::MeshData>(model_data[i]);
        const auto& [present, albedo, normals, metallic_roughness, emissive] = std::get<gfx::MaterialData>(model_data[i]);

        Buffer vertex_buffer(context, vertices.size() * sizeof(VertexP3N3U2T4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, vertices.data());
        Buffer index_buffer(context, indices.size() * sizeof(VertexIndexType), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, indices.data());

        MaterialEntry material_entry{};

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

            const auto albedo_id = TextureId{ next_text_id++ };
            texture_map.insert({ albedo_id, std::move(albedo_text) });

            material_entry.albedo = albedo_id;
        }
        if ((present & MaterialFlag::Normal) != MaterialFlag::None)
        {
            Buffer normal_buff(context, normals.width * normals.height * normals.channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, normals.data.data());

            Texture normal_text(context, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                                0, static_cast<u32>(normals.width), static_cast<u32>(normals.height), 1, 1, 1);

            normal_text.create_sampler();

            VkBufferImageCopy image_copy{};
            image_copy.imageSubresource = VkImageSubresourceLayers{ normal_text.aspect, 0, 0, 1 };
            image_copy.imageExtent = normal_text.extent;
            image_copy.imageOffset = { 0, 0, 0 };
            image_copy.bufferOffset = 0;

            copy_buffer_to_texture(normal_buff, normal_text, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { image_copy });

            const auto normal_id = TextureId{ next_text_id++ };
            texture_map.insert({ normal_id, std::move(normal_text) });

            material_entry.normal = normal_id;
        }
        if ((present & MaterialFlag::MetallicRoughness) != MaterialFlag::None)
        {
            Buffer metallic_roughness_buff(context, metallic_roughness.width * metallic_roughness.height * metallic_roughness.channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, metallic_roughness.data.data());

            Texture metallic_roughness_text(context, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                                0, static_cast<u32>(metallic_roughness.width), static_cast<u32>(metallic_roughness.height), 1, 1, 1);

            metallic_roughness_text.create_sampler();

            VkBufferImageCopy image_copy{};
            image_copy.imageSubresource = VkImageSubresourceLayers{ metallic_roughness_text.aspect, 0, 0, 1 };
            image_copy.imageExtent = metallic_roughness_text.extent;
            image_copy.imageOffset = { 0, 0, 0 };
            image_copy.bufferOffset = 0;

            copy_buffer_to_texture(metallic_roughness_buff, metallic_roughness_text, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { image_copy });

            const auto metallic_roughness_id = TextureId{ next_text_id++ };
            texture_map.insert({ metallic_roughness_id, std::move(metallic_roughness_text) });

            material_entry.metallic_roughness = metallic_roughness_id;
        }
        if ((present & MaterialFlag::Emissive) != MaterialFlag::None)
        {
            Buffer emissive_buff(context, emissive.width * emissive.height * emissive.channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, emissive.data.data());

            Texture emissive_text(context, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                                  VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                                  0, static_cast<u32>(emissive.width), static_cast<u32>(emissive.height), 1, 1, 1);

            emissive_text.create_sampler();

            VkBufferImageCopy image_copy{};
            image_copy.imageSubresource = VkImageSubresourceLayers{ emissive_text.aspect, 0, 0, 1 };
            image_copy.imageExtent = emissive_text.extent;
            image_copy.imageOffset = { 0, 0, 0 };
            image_copy.bufferOffset = 0;

            copy_buffer_to_texture(emissive_buff, emissive_text, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { image_copy });

            const auto emissive_id = TextureId{ next_text_id++ };
            texture_map.insert({ emissive_id, std::move(emissive_text) });

            material_entry.emissive = emissive_id;
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

void ResourceManager::copy_buffer_to_texture(const Buffer& buffer, Texture& texture, const VkImageLayout new_layout, const std::vector<VkBufferImageCopy>& regions) const
{
    const auto cmd = command.begin();
    texture.set_layout_cmd(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    vkCmdCopyBufferToImage(cmd, buffer.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<u32>(regions.size()), regions.data());
    texture.set_layout_cmd(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
    command.flush();
}
}
