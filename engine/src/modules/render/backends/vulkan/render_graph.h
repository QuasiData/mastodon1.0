#pragma once
#include "vk_context.h"
#include "resources/vk_resource_manager.h"

namespace mas::gfx::vulkan
{
class RenderGraph;

class RenderNode
{
public:
    RenderNode() = default;
    virtual ~RenderNode() = default;
    DISABLE_COPY_AND_MOVE(RenderNode)

    virtual void setup_resources(const std::shared_ptr<Context>& context, const ResourceManager& resource_manager) {}
};

class RenderGraph
{
public:
    explicit RenderGraph(std::shared_ptr<Context> c)
    {
        context = std::move(c);
    }
    ~RenderGraph() = default;
    DISABLE_COPY_AND_MOVE(RenderGraph)

    void run(const ResourceManager& res)
    {
        for (auto& node: nodes)
        {
            node->setup_resources(context, res);
        }
    }

    void add_node(std::unique_ptr<RenderNode> node)
    {
        nodes.emplace_back(std::move(node));
    }

private:
    std::shared_ptr<Context> context{ nullptr };

    std::vector<std::unique_ptr<RenderNode>> nodes{};
};
}