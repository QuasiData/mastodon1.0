#pragma once
#include "common.h"

#include "flecs/flecs.h"

#include <string>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

namespace mas
{
class App;

struct WindowSettings
{
    u32 width{ 1920 };
    u32 height{ 1080 };
    std::string title{ "Mastodon" };
};

class Window
{
    GLFWwindow* window{ nullptr };

    void init(const WindowSettings& settings);
    [[nodiscard]] bool should_close() const;
    void poll_events() const;

    friend class App;

public:
    Window() = default;
    ~Window();
    DISABLE_COPY(Window)
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;

    [[nodiscard]] GLFWwindow* get_raw_window() const;
    [[nodiscard]] std::pair<u32, u32> inner_size() const;
    void enable_cursor() const;
    void disable_cursor() const;
};

struct WindowModule
{
    // ReSharper disable once CppNonExplicitConvertingConstructor
    WindowModule(const flecs::world& world);
};
}
