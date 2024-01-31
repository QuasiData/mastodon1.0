#pragma once
#include "vk_context.h"
#include "resources/vk_resource_manager.h"

#include <string>

namespace mas::gfx::vulkan
{
class RenderGraph;

class RenderNode
{
public:
    RenderNode() = default;
    virtual ~RenderNode() = default;
    DISABLE_COPY_AND_MOVE(RenderNode)

    // Create shader resources. Such as storage buffers or uniforms
    virtual void setup_resources(const std::shared_ptr<Context>& context, ResourceManager& resource_manager) {}

    // Get shader resources created by other nodes and descriptor supplied by the engine, such as the texture array descriptor.
    virtual void ready_resources(const std::shared_ptr<Context>& context, ResourceManager& resource_manager) {}

    // Update shader resources. For example update buffer with data from cpu. Only update resources created in this node.
    virtual void update_resources(const std::shared_ptr<Context>& context, ResourceManager& resource_manager, flecs::world* world) {}

    // Create pipeline and any descriptors.
    virtual void setup(const std::shared_ptr<Context>& context, ResourceManager& resource_manager) {}

    // Record commands to the command buffer for execution.
    virtual void run(VkCommandBuffer cmd, const std::shared_ptr<Context>& context, ResourceManager& resource_manager, flecs::world* world) {}

    // Use Imgui to create ui for this node.
    virtual void draw_ui(flecs::world* world) {}

};

enum class RenderPassType
{
    Render,
    Compute,
};

class RenderGraph
{
    struct RenderPass
    {
        RenderPass(RenderPassType type) : type(type) {}
        ~RenderPass() = default;
        DISABLE_COPY(RenderPass)
        RenderPass(RenderPass&& other) = default;
        RenderPass& operator=(RenderPass&&) = default;

        RenderPassType type;
        std::vector<std::unique_ptr<RenderNode>> nodes;
    };

public:
    explicit RenderGraph(std::shared_ptr<Context> c);
    ~RenderGraph() = default;
    DISABLE_COPY_AND_MOVE(RenderGraph)

    void setup_node_resources(ResourceManager& res) const;

    void ready_node_resources(ResourceManager& res) const;

    void update_node_resources(ResourceManager& res, flecs::world* world) const;

    void setup_nodes(ResourceManager& res) const;

    void run(const VkCommandBuffer cmd, ResourceManager& res, flecs::world* world) const;

    void draw_ui(flecs::world* world) const;

    void add_node(std::unique_ptr<RenderNode> node, const std::string& name, const std::string& pass);

    void add_node_edge(const std::string& from, const std::string& to);

    void add_pass(const std::string& name, RenderPassType type);

    void add_pass_edge(const std::string& from, const std::string& to);

    void setup();

private:
    std::shared_ptr<Context> context{ nullptr };

    std::vector<RenderPass> passes;

    std::unordered_map<std::string, usize> named_nodes;
    std::unordered_map<std::string, usize> named_passes;

    std::unordered_map<std::string, std::unique_ptr<RenderNode>> nodes_to_add;
    std::unordered_map<std::string, std::string> node_to_pass_add_map;
    std::unordered_map<std::string, RenderPass> passes_to_add;

    std::unordered_map<std::string, std::vector<std::string>> node_edges_to_create;
    std::unordered_map<std::string, std::vector<std::string>> pass_edges_to_create;

    std::unordered_map<usize, RenderPass> pass_map;
    std::unordered_map<usize, std::unique_ptr<RenderNode>> node_map;
    std::unordered_map<usize, usize> node_to_pass;
};
}