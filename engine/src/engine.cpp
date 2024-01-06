#include "engine.h"
#include "modules/input/input_module.h"

#include "spdlog/spdlog.h"

#include <chrono>

namespace mas
{
App::App(const AppSettings& settings)
{
    // Import all modules
    world.import<WindowModule>();
    world.import<InputModule>();

    // Init resources
    world.set(Window{});
    world.get_mut<Window>()->init(settings.window_settings);

    world.set(KeyboardInput{});
    world.set(MouseInput{});
}

void App::run()
{
    spdlog::info("Application launching");
    f32 frame_time{ 0 };
    const auto window = world.get<Window>();

    while(!window->should_close())
    {
        const auto start_time = std::chrono::high_resolution_clock::now();
        window->poll_events();

        if (!world.progress(frame_time))
            spdlog::error("Failed to progress world");

        const auto end_time = std::chrono::high_resolution_clock::now();
        frame_time = std::chrono::duration<f32>(end_time - start_time).count();
    }

    close();
}

void App::close()
{
    spdlog::info("Closing application");
}
}
