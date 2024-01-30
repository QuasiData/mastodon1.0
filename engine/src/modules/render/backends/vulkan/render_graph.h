#pragma once
#include "vk_context.h"

namespace mas::gfx::vulkan
{
class RenderNode
{
    
};

class RenderGraph
{

private:
    std::vector<std::unique_ptr<RenderNode>> nodes{};
};
}