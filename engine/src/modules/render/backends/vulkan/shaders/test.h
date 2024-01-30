#pragma once
#include "../vk_context.h"
#include "../render_graph.h"
#include "../resources/vk_resource_manager.h"

namespace mas::gfx::vulkan
{
class TestNode final: public RenderNode
{
public:
    void setup_resources(const std::shared_ptr<Context>& context, const ResourceManager& resource_manager) override;
};
}