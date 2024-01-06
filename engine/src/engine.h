#pragma once
#include "common.h"

#include "modules/window/window_module.h"

#include "flecs/flecs.h"

namespace mas
{
struct AppSettings
{
    WindowSettings window_settings{};
};

class App
{
public:
    flecs::world world{};
    explicit App(const AppSettings& settings);
    void run();

private:
    void close();
};
}
