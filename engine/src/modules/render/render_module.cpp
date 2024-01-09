#include "render_module.h"
#include "modules/window/window_module.h"

#include <stdexcept>


namespace mas
{
RenderModule::RenderModule(flecs::world& world)
{
    if (const auto result = world.module<RenderModule>(); !result)
        throw std::runtime_error("Failed to add render module!");

    world.import<WindowModule>();

    // TODO: System OnLoad that starts a new frame for UI rendering this frame

    world.system<Renderer>("Render system")
        .term_at(1).singleton()
        .kind(flecs::OnStore)
        .each([&world](const Renderer& r)
              {
                  r->render(&world);
              });
}
}
