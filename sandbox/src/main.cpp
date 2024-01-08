#include <iostream>

#include "engine.h"
#include "modules/render/render_module.h"

int main()
{
    const auto settings = mas::AppSettings{
        .window_settings = mas::WindowSettings {
            .width = 1920,
            .height = 1080,
            .title = "Mastodon Engine"
        },
    };
    auto app = mas::App(settings);

    for (usize i{ 0 }; i < 10000000; ++i)
    {
        app.world.entity().set(mas::Transform{}).set(mas::GlobalTransform{});
    }

    app.run();
    return 0;
}
