#pragma once
#include "../vk_context.h"

namespace mas::gfx::vulkan
{
DEFINE_TYPED_ID(Texture)

class Texture
{
public:
    Texture() = delete;
    ~Texture();
    DISABLE_COPY(Texture)
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    Texture(std::shared_ptr<Context> c, VkImageType image_type, VkFormat format,
            VkImageAspectFlags aspect, VkImageUsageFlags usage,VkImageTiling tiling, VkSampleCountFlagBits samples,
            VkImageCreateFlags flags, u32 width, u32 height, u32 depth, u32 mip_levels, u32 array_layers);

    void create_sampler(VkFilter mag_filter = VK_FILTER_LINEAR, VkFilter min_filter = VK_FILTER_LINEAR,
                        VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                        VkSamplerAddressMode u_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                        VkSamplerAddressMode v_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                        VkSamplerAddressMode w_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                        f32 mip_lod_bias = 0, VkCompareOp compare_op = VK_COMPARE_OP_NEVER,
                        f32 min_lod = 0, f32 max_lod = -1.0f,
                        VkBorderColor border_color = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, f32 anisotropy = 1.0f);

    void set_layout_now(VkImageLayout new_layout, VkPipelineStageFlagBits2 src_stage, VkPipelineStageFlagBits2 dst_stage);

    void set_layout_cmd(VkCommandBuffer cmd, VkImageLayout new_layout, VkPipelineStageFlagBits2 src_stage, VkPipelineStageFlagBits2 dst_stage);

private:
    std::shared_ptr<Context> context{ nullptr };

public:
    VkImage image{ nullptr };
    VkImageView image_view{ nullptr };
    VkSampler sampler{ nullptr };
    VmaAllocation allocation{ nullptr };
    VmaAllocationInfo allocation_info{};
    VkFormat format;
    VkImageLayout layout;
    VkImageAspectFlags aspect;
    u32 mip_levels{ 1 };
    u32 array_layers{ 1 };
    VkExtent3D extent;

    bool cube;
};
}