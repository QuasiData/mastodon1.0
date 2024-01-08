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
    friend class App;
public:
    Window() = default;
    ~Window();
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;
    DISABLE_COPY(Window)

    [[nodiscard]] GLFWwindow* get_raw_window() const;

    [[nodiscard]] std::pair<u32, u32> inner_size() const;

    void enable_cursor() const;

    void disable_cursor() const;

private:
    GLFWwindow* window{ nullptr };

    void init(const WindowSettings& settings);

    [[nodiscard]] bool should_close() const;

    void poll_events() const;
};

struct WindowModule
{
    // ReSharper disable once CppNonExplicitConvertingConstructor
    WindowModule(const flecs::world& world);
};
}
