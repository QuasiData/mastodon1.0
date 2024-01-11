#include "vk_ui.h"
#include "vk_command.h"

#include "spdlog/spdlog.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#include <stdexcept>

namespace mas::gfx::vulkan
{
UiOverlay::~UiOverlay()
{
	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(context->device, imgui_pool, nullptr);
}

UiOverlay::UiOverlay(std::shared_ptr<Context> c)
{
    context = std::move(c);
    const VkDescriptorPoolSize pool_sizes[] =
    {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = static_cast<u32>(std::size(pool_sizes));
	pool_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(context->device, &pool_info, nullptr, &imgui_pool) != VK_SUCCESS)
	{
		spdlog::error("Failed to create imgui descriptor pool");
		throw std::runtime_error("Failed to create imgui descriptor pool");
	}

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(context->window, true);
	ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance)
								   {
									   return vkGetInstanceProcAddr(*(static_cast<VkInstance*>(vulkan_instance)), function_name);
								   }, &context->instance);

	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = context->instance;
	init_info.PhysicalDevice = context->phys_device;
	init_info.Device = context->device;
	init_info.Queue = context->graphics_queue;
	init_info.DescriptorPool = imgui_pool;
	init_info.MinImageCount = back_buffer_count;
	init_info.ImageCount = back_buffer_count;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.UseDynamicRendering = true;
	init_info.ColorAttachmentFormat = context->surface_format.format;

	ImGui_ImplVulkan_Init(&init_info, VK_NULL_HANDLE);
	ImGui_ImplVulkan_CreateFontsTexture();

	ImGui::StyleColorsDark();
}

void UiOverlay::new_frame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UiOverlay::end_frame()
{
	ImGui::Render();
}

void UiOverlay::draw_cmd(const VkCommandBuffer cmd, const VkImageView target_view) const
{
	VkRenderingAttachmentInfo color{};
	color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	color.imageView = target_view;
	color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.clearValue = VkClearValue{ .color = { { 0, 0, 0 } } };

	VkRenderingInfo render_info{};
	render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	render_info.renderArea = VkRect2D{ .offset = {0, 0}, .extent = context->surface_extent };
	render_info.layerCount = 1;
	render_info.pColorAttachments = &color;
	render_info.colorAttachmentCount = 1;

	vkCmdBeginRendering(cmd, &render_info);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	vkCmdEndRendering(cmd);
}
}
