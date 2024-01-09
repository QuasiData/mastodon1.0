#include <iostream>

#include "engine.h"
#include "modules/asset/asset_loader.h"
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

    for (usize i{ 0 }; i < 100; ++i)
    {
        app.world.entity().set(mas::Transform{}).set(mas::GlobalTransform{});
    }

    app.world.system<mas::AssetLoader>()
        .kind(flecs::OnStart)
        .term_at(1).singleton()
        .each([](mas::AssetLoader& al)
              {
                  al.loaf_glb("./assets/DamagedHelmet.glb");
              });

        app.run();
        return 0;
}
