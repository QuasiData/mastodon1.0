#include "engine.h"
#include "modules/input/input_module.h"
#include "modules/render/render_module.h"
#include "modules/asset/asset_loader.h"
#include "modules/render/backends/vulkan/vk_renderer.h"

#include "spdlog/spdlog.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <chrono>
#include <thread>

namespace mas
{
App::App(const AppSettings& settings)
{
    const auto threads = std::thread::hardware_concurrency();
    spdlog::info("Cores detected: {}", threads);
    world.set_threads(static_cast<i32>(threads));

    // Import all modules
    world.import<TransformModule>();
    world.import<WindowModule>();
    world.import<InputModule>();
    world.import<RenderModule>();

    // Init resources
    world.set(Window{});
    world.get_mut<Window>()->init(settings.window_settings);

    world.set(KeyboardInput{});
    world.set(MouseInput{});

    world.set<Renderer>(std::make_shared<gfx::vulkan::Renderer>(world.get<Window>()->get_raw_window(), &world));
    world.set(AssetLoader{});

    world.get_mut<AssetLoader>()->inject_renderer(*world.get_mut<Renderer>());
}

void App::run() const
{
    spdlog::info("Application launching");
    f32 frame_time{ 0 };
    const auto window = world.get<Window>();

    bool startup{ true };
    while(!window->should_close())
    {
        const auto start_time = std::chrono::high_resolution_clock::now();
        window->poll_events();

        if (!world.progress(frame_time))
            spdlog::error("Failed to progress world");

        if (startup)
        {
            const auto asset_loader = world.get_mut<AssetLoader>();
            asset_loader->upload_all();
            asset_loader->startup = false;
            startup = false;
        }

        const auto end_time = std::chrono::high_resolution_clock::now();
        frame_time = std::chrono::duration<f32>(end_time - start_time).count();
    }

    close();
}

void App::close() const
{
    spdlog::info("Closing application");
    world.remove<AssetLoader>();
    world.remove<Renderer>();
    world.remove<Window>();
}
}
