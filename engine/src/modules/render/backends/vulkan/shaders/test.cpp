#include "test.h"

#include "spdlog/spdlog.h"

namespace mas::gfx::vulkan
{
void TestNode::setup_resources(const std::shared_ptr<Context>& context, ResourceManager& resource_manager)
{
    auto buffer = Buffer(context, 1000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer.set_debug_name("TestBuffer");

    const auto res = resource_manager.add_buffer(std::move(buffer), "TestBuffer");
}

void PlutNode::setup_resources(const std::shared_ptr<Context>& context, ResourceManager& resource_manager)
{
    auto buffer = Buffer(context, 1000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer.set_debug_name("PlutBuffer");

    const auto res = resource_manager.add_buffer(std::move(buffer), "PlutBuffer");
}
}
