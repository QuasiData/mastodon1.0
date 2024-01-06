#include <iostream>

#include "engine.h"

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
    app.run();
    return 0;
}