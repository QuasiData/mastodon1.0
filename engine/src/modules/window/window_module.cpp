#include "window_module.h"

#include "glfw/glfw3.h"
#include "spdlog/spdlog.h"

#include <stdexcept>

namespace mas
{
void Window::init(const WindowSettings& settings)
{
    spdlog::info("Initializing glfw window");

    if (!glfwInit())
        throw std::runtime_error("Failed to initialize glfw!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(static_cast<i32>(settings.width), static_cast<i32>(settings.height), settings.title.c_str(), nullptr, nullptr);

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // TODO: re-enable this default
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create glfw window!");
    }
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(window);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void Window::poll_events() const
{
    glfwPollEvents();
}

Window::~Window()
{
    glfwTerminate();
}

Window::Window(Window&& other) noexcept
{
    *this = std::move(other);
}

Window& Window::operator=(Window&& other) noexcept
{
    window = other.window;
    other.window = nullptr;
    return *this;
}

GLFWwindow* Window::get_raw_window() const
{
    return window;
}

std::pair<u32, u32> Window::inner_size() const
{
    i32 width{ 0 };
    i32 height{ 0 };
    glfwGetFramebufferSize(window, &width, &height);
    return std::make_pair(static_cast<u32>(width), static_cast<u32>(height));
}

void Window::enable_cursor() const
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::disable_cursor() const
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

WindowModule::WindowModule(const flecs::world& world)
{
    if (const auto result = world.module<WindowModule>(); !result)
        throw std::runtime_error("Failed to add window module!");
}
}
