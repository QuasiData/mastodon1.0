// ReSharper disable CppClangTidyBugproneBranchClone
#include "vk_texture.h"
#include "modules/render/backends/vulkan/vk_command.h"

#include "spdlog/spdlog.h"
#include "glm/ext/matrix_common.hpp"

#include <stdexcept>

namespace mas::gfx::vulkan
{
Texture::~Texture()
{
    if (image)
    {
        vmaDestroyImage(context->allocator, image, allocation);
        image = nullptr;
    }

    if (image_view)
    {
        vkDestroyImageView(context->device, image_view, nullptr);
        image_view = nullptr;
    }

    if (sampler)
    {
        vkDestroySampler(context->device, sampler, nullptr);
        sampler = nullptr;
    }

    allocation = nullptr;
    context = nullptr;
}

Texture::Texture(Texture&& other) noexcept
{
    *this = std::move(other);
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    this->context = other.context;
    other.context = nullptr;

    this->image = other.image;
    other.image = nullptr;

    this->image_view = other.image_view;
    other.image_view = nullptr;

    this->allocation = other.allocation;
    other.allocation = nullptr;

    this->allocation_info = other.allocation_info;
    other.allocation_info = VmaAllocationInfo{};

    this->sampler = other.sampler;
    other.sampler = nullptr;

    this->format = other.format;
    this->aspect = other.aspect;
    this->mip_levels = other.mip_levels;
    this->array_layers = other.array_layers;
    this->extent = other.extent;
    this->cube = other.cube;

    return *this;
}

Texture::Texture(
    std::shared_ptr<Context> c, const VkImageType image_type, const VkFormat format,
    const VkImageAspectFlags aspect, const VkImageUsageFlags usage,
    const VkImageTiling tiling, const VkSampleCountFlagBits samples, const VkImageCreateFlags flags,
    const u32 width, const u32 height, const u32 depth, const u32 mip_levels, const u32 array_layers)
{
    context = std::move(c);
    this->format = format;
    this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    this->aspect = aspect;
    extent = VkExtent3D{ width, height, depth };
    this->mip_levels = mip_levels;
    this->array_layers = array_layers;
    cube = flags == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    if (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
    {
        if (array_layers != 6)
        {
            spdlog::error("Cube compatible requires array layers to be 6, but array layers was: {}", array_layers);
            throw std::runtime_error("Failed to create cube map");
        }
    }

    VkImageCreateInfo image_ci{};
    image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_ci.imageType = image_type;
    image_ci.format = format;
    image_ci.tiling = tiling;
    image_ci.initialLayout = layout;
    image_ci.usage = usage;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.samples = samples;
    image_ci.flags = flags;
    image_ci.extent = VkExtent3D{ width, height, depth };
    image_ci.mipLevels = mip_levels;
    image_ci.arrayLayers = array_layers;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

    if (vmaCreateImage(context->allocator, &image_ci, &alloc_info, &image, &allocation, &allocation_info) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image");

    const VkImageViewType view_type = flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

    VkImageViewCreateInfo view_ci{};
    view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_ci.viewType = view_type;
    view_ci.format = format;
    view_ci.subresourceRange = VkImageSubresourceRange{ aspect, 0, mip_levels, 0, array_layers };
    view_ci.image = image;

    if (vkCreateImageView(context->device, &view_ci, nullptr, &image_view) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");
}

void Texture::create_sampler(
    const VkFilter mag_filter, const VkFilter min_filter, const VkSamplerMipmapMode mipmap_mode,
    const VkSamplerAddressMode u_mode, const VkSamplerAddressMode v_mode, const VkSamplerAddressMode w_mode,
    const f32 mip_lod_bias, const VkCompareOp compare_op, const f32 min_lod, const f32 max_lod,
    const VkBorderColor border_color, const f32 anisotropy)
{
    VkSamplerCreateInfo sampler_ci{};
    sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_ci.magFilter = mag_filter;
    sampler_ci.minFilter = min_filter;
    sampler_ci.mipmapMode = mipmap_mode;
    sampler_ci.addressModeU = u_mode;
    sampler_ci.addressModeV = v_mode;
    sampler_ci.addressModeW = w_mode;
    sampler_ci.mipLodBias = mip_lod_bias;
    sampler_ci.compareOp = compare_op;
    sampler_ci.minLod = min_lod;
    sampler_ci.maxLod = glm::abs(max_lod - (-1.0f)) < 0.0001f ? static_cast<f32>(mip_levels) : max_lod;
    sampler_ci.borderColor = border_color;
    sampler_ci.maxAnisotropy = anisotropy;

    if (vkCreateSampler(context->device, &sampler_ci, nullptr, &sampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create sampler");
}

namespace
{
void get_access(const VkImageLayout current_layout, const VkImageLayout new_layout, VkAccessFlags2& src_access, VkAccessFlags2& dst_access)
{
    if (current_layout == VK_IMAGE_LAYOUT_UNDEFINED)
        src_access = 0;
    else if (current_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        src_access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    else if (current_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        src_access = VK_ACCESS_2_TRANSFER_READ_BIT;
    else if (current_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        src_access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    else if (current_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        src_access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    else if (current_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)
        src_access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    else if (current_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        src_access = VK_ACCESS_2_SHADER_READ_BIT;
    else
    {
        throw std::runtime_error("Not supported layout");
    }


    if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        dst_access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        dst_access = VK_ACCESS_2_TRANSFER_READ_BIT;
    else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        dst_access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    else if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        dst_access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    else if (new_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)
        dst_access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    else if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        dst_access = VK_ACCESS_2_SHADER_READ_BIT;
    else
    {
        throw std::runtime_error("Not supported layout");
    }
}
}

void Texture::set_layout_now(const VkImageLayout new_layout, const VkPipelineStageFlagBits2 src_stage, const VkPipelineStageFlagBits2 dst_stage)
{
    const Command command(context, context->graphics_queue, context->queue_family_indices.graphics_family.value());
    const auto cmd = command.begin();

    VkAccessFlags2 src_access{};
    VkAccessFlags2 dst_access{};
    get_access(layout, new_layout, src_access, dst_access);

    const VkImageSubresourceRange s_range{ aspect, 0, mip_levels, 0, array_layers };

    VkImageMemoryBarrier2 image_barrier{};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    image_barrier.image = image;
    image_barrier.subresourceRange = s_range;
    image_barrier.srcStageMask = src_stage;
    image_barrier.srcAccessMask = src_access;
    image_barrier.dstStageMask = dst_stage;
    image_barrier.dstAccessMask = dst_access;
    image_barrier.oldLayout = layout;
    image_barrier.newLayout = new_layout;

    VkDependencyInfo dep_info{};
    dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep_info.pImageMemoryBarriers = &image_barrier;
    dep_info.imageMemoryBarrierCount = 1;

    vkCmdPipelineBarrier2(cmd, &dep_info);
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pCommandBuffers = &cmd;
    submit_info.commandBufferCount = 1;

    vkQueueSubmit(context->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphics_queue);
    vkResetCommandBuffer(cmd, 0);

    layout = new_layout;
}

void Texture::set_layout_cmd(const VkCommandBuffer cmd, const VkImageLayout new_layout, const VkPipelineStageFlagBits2 src_stage, const VkPipelineStageFlagBits2 dst_stage)
{
    VkAccessFlags2 src_access{};
    VkAccessFlags2 dst_access{};
    get_access(layout, new_layout, src_access, dst_access);

    const VkImageSubresourceRange s_range{ aspect, 0, mip_levels, 0, array_layers };

    VkImageMemoryBarrier2 image_barrier{};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    image_barrier.image = image;
    image_barrier.subresourceRange = s_range;
    image_barrier.srcStageMask = src_stage;
    image_barrier.srcAccessMask = src_access;
    image_barrier.dstStageMask = dst_stage;
    image_barrier.dstAccessMask = dst_access;
    image_barrier.oldLayout = layout;
    image_barrier.newLayout = new_layout;

    VkDependencyInfo dep_info{};
    dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep_info.pImageMemoryBarriers = &image_barrier;
    dep_info.imageMemoryBarrierCount = 1;

    vkCmdPipelineBarrier2(cmd, &dep_info);
    layout = new_layout;
}

void Texture::set_debug_name(const std::string& name)
{
    if constexpr (enable_validation)
    {
        auto name_info = VkDebugUtilsObjectNameInfoEXT{};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        name_info.objectHandle = reinterpret_cast<u64>(image);
        name_info.objectType = VK_OBJECT_TYPE_BUFFER;

        std::string ext = " image";
        name_info.pObjectName = (name + ext).c_str();

        vkSetDebugUtilsObjectNameEXT(context->device, &name_info);

        name_info = VkDebugUtilsObjectNameInfoEXT{};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        name_info.objectHandle = reinterpret_cast<u64>(image_view);
        name_info.objectType = VK_OBJECT_TYPE_BUFFER;

        ext = " view";
        name_info.pObjectName = (name + ext).c_str();

        vkSetDebugUtilsObjectNameEXT(context->device, &name_info);

        name_info = VkDebugUtilsObjectNameInfoEXT{};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        name_info.objectHandle = reinterpret_cast<u64>(sampler);
        name_info.objectType = VK_OBJECT_TYPE_BUFFER;

        ext = " sampler";
        name_info.pObjectName = (name + ext).c_str();

        vkSetDebugUtilsObjectNameEXT(context->device, &name_info);
    }
}
}
