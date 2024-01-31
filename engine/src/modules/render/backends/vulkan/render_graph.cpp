#include "render_graph.h"

#include <ranges>

namespace mas::gfx::vulkan
{
namespace
{
void topological_sort_util(const usize i, std::vector<bool>& visited, std::vector<usize>& stack, const std::vector<std::vector<usize>>& adj_vec)
{
    visited[i] = true;
    for (usize j : adj_vec[i])
    {
        if (!visited[j])
        {
            topological_sort_util(j, visited, stack, adj_vec);
        }
    }

    stack.push_back(i);
}

std::vector<usize> topological_sort(const std::vector<std::vector<usize>>& adj_vec)
{
    std::vector<usize> stack{};
    std::vector<bool> visited(adj_vec.size());

    for (auto&& b : visited)
    {
        b = false;
    }

    for (usize i{ 0 }; i < visited.size(); ++i)
    {
        if (!visited[i])
        {
            topological_sort_util(i, visited, stack, adj_vec);
        }
    }

    std::reverse(stack.begin(), stack.end());
    return stack; 
}
}

RenderGraph::RenderGraph(std::shared_ptr<Context> c)
{
    context = std::move(c);
}

void RenderGraph::setup_node_resources(ResourceManager& res) const
{
    for (const auto& [type, nodes] : passes)
    {
        for (const auto& node : nodes)
        {
            node->setup_resources(context, res);
        }
    }
}

void RenderGraph::ready_node_resources(ResourceManager& res) const
{
    for (const auto& [type, nodes] : passes)
    {
        for (const auto& node : nodes)
        {
            node->ready_resources(context, res);
        }
    }
}

void RenderGraph::update_node_resources(ResourceManager& res, flecs::world* world) const
{
    for (const auto& [type, nodes] : passes)
    {
        for (const auto& node : nodes)
        {
            node->update_resources(context, res, world);
        }
    }
}

void RenderGraph::setup_nodes(ResourceManager& res) const
{
    for (const auto& [type, nodes] : passes)
    {
        for (const auto& node : nodes)
        {
            node->setup(context, res);
        }
    }
}

void RenderGraph::run(const VkCommandBuffer cmd, ResourceManager& res, flecs::world* world) const
{
    for (const auto& [type, nodes] : passes)
    {
        for (const auto& node : nodes)
        {
            node->run(cmd, context, res, world);
        }
    }
}

void RenderGraph::draw_ui(flecs::world* world) const
{
    for (const auto& [type, nodes] : passes)
    {
        for (const auto& node : nodes)
        {
            node->draw_ui(world);
        }
    }
}

void RenderGraph::add_node(std::unique_ptr<RenderNode> node, const std::string& name, const std::string& pass)
{
    nodes_to_add.insert({ name, std::move(node) });
    node_to_pass_add_map.insert({ name, pass });
}

void RenderGraph::add_node_edge(const std::string& from, const std::string& to)
{
    if (!node_edges_to_create.contains(from))
    {
        node_edges_to_create.insert({ from, {to} });
    }
    else
    {
        auto& edges = node_edges_to_create.at(from);
        edges.emplace_back(to);
    }
}

void RenderGraph::add_pass(const std::string& name, const RenderPassType type)
{
    passes_to_add.insert({ name, RenderPass(type) });
}

void RenderGraph::add_pass_edge(const std::string& from, const std::string& to)
{
    if (!pass_edges_to_create.contains(from))
    {
        pass_edges_to_create.insert({ from, {to} });
    }
    else
    {
        auto& edges = pass_edges_to_create.at(from);
        edges.emplace_back(to);
    }
}

void RenderGraph::setup()
{
    usize next_pass_id{ 0 };
    std::vector<std::vector<usize>> render_passes(passes_to_add.size());
    for (const auto& key: passes_to_add | std::views::keys)
    {
        const auto pass_id = usize{ next_pass_id++ };

        RenderPass pass = std::move(passes_to_add.at(key));

        pass_map.insert({ pass_id, std::move(pass) });
        named_passes.insert({ key, pass_id });
    }
    passes_to_add.clear();

    for (const auto& [key, value]: pass_edges_to_create)
    {
        for (const auto& name: value)
        {
            const auto pass_id = named_passes.at(key);
            render_passes[pass_id].emplace_back(named_passes.at(name));
        }
    }
    pass_edges_to_create.clear();

    usize next_node_id{ 0 };
    std::vector<std::vector<usize>> render_nodes(nodes_to_add.size());
    for (const auto& key : nodes_to_add | std::views::keys)
    {
        const auto& pass_name = node_to_pass_add_map.at(key);
        const auto& pass_id = named_passes.at(pass_name);

        const auto node_id = usize{ next_node_id++ };
        std::unique_ptr<RenderNode> node = std::move(nodes_to_add.at(key));

        node_map.insert({ node_id, std::move(node) });
        named_nodes.insert({ key, node_id });
        node_to_pass.insert({ node_id, pass_id });
    }
    nodes_to_add.clear();

    for (const auto& [key, value] : node_edges_to_create)
    {
        for (const auto& name : value)
        {
            const auto node_id = named_nodes.at(key);
            const auto pass_id = node_to_pass.at(node_id);
            render_nodes[node_id].emplace_back(named_nodes.at(name));

        }
    }

    const auto pass = topological_sort(render_passes);
    const auto nodes = topological_sort(render_nodes);
    for (usize i : pass)
    {
        RenderPass p = std::move(pass_map.at(i));

        for (usize j : nodes)
        {
            if (node_to_pass.at(j) == i)
            {
                std::unique_ptr<RenderNode> n = std::move(node_map.at(j));
                p.nodes.emplace_back(std::move(n));
            }
        }

        passes.emplace_back(std::move(p));
    }
    pass_map.clear();
    node_map.clear();
    node_to_pass.clear();
}
}
