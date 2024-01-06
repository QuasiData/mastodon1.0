#include "input_module.h"
#include "modules/window/window_module.h"

#include "glfw/glfw3.h"
#include "spdlog/spdlog.h"

#include <stdexcept>

#define MAS_CONVERT_INPUT(flag, dst, src)                                       \
    {                                                                           \
        const auto action = glfwGetKey(window, (src));                          \
        if (action == GLFW_PRESS)                                               \
            (flag) |= (dst);                                                    \
        else if (action == GLFW_RELEASE)                                        \
            (flag) &= ~(dst);                                                   \
    }

namespace mas
{
bool KeyboardInput::pressed_internal(const KeyCode input, const KeyCode compare)
{
    if ((input & compare) != KeyCode::None)
        return true;
    return false;
}

bool KeyboardInput::pressed(const KeyCode input) const
{
    return pressed_internal(input, flag);
}

bool KeyboardInput::just_pressed(const KeyCode input) const
{
    return pressed_internal(input, prev_flag);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void KeyboardInput::convert_glfw_input(GLFWwindow* window)
{
    prev_flag = flag;
    MAS_CONVERT_INPUT(flag, KeyCode::A, GLFW_KEY_A);
    MAS_CONVERT_INPUT(flag, KeyCode::B, GLFW_KEY_B);
    MAS_CONVERT_INPUT(flag, KeyCode::C, GLFW_KEY_C);
    MAS_CONVERT_INPUT(flag, KeyCode::D, GLFW_KEY_D);
    MAS_CONVERT_INPUT(flag, KeyCode::E, GLFW_KEY_E);
    MAS_CONVERT_INPUT(flag, KeyCode::F, GLFW_KEY_F);
    MAS_CONVERT_INPUT(flag, KeyCode::G, GLFW_KEY_G);
    MAS_CONVERT_INPUT(flag, KeyCode::H, GLFW_KEY_H);
    MAS_CONVERT_INPUT(flag, KeyCode::I, GLFW_KEY_I);
    MAS_CONVERT_INPUT(flag, KeyCode::J, GLFW_KEY_J);
    MAS_CONVERT_INPUT(flag, KeyCode::K, GLFW_KEY_K);
    MAS_CONVERT_INPUT(flag, KeyCode::L, GLFW_KEY_L);
    MAS_CONVERT_INPUT(flag, KeyCode::M, GLFW_KEY_M);
    MAS_CONVERT_INPUT(flag, KeyCode::N, GLFW_KEY_N);
    MAS_CONVERT_INPUT(flag, KeyCode::O, GLFW_KEY_O);
    MAS_CONVERT_INPUT(flag, KeyCode::P, GLFW_KEY_P);
    MAS_CONVERT_INPUT(flag, KeyCode::Q, GLFW_KEY_Q);
    MAS_CONVERT_INPUT(flag, KeyCode::R, GLFW_KEY_R);
    MAS_CONVERT_INPUT(flag, KeyCode::S, GLFW_KEY_S);
    MAS_CONVERT_INPUT(flag, KeyCode::T, GLFW_KEY_T);
    MAS_CONVERT_INPUT(flag, KeyCode::U, GLFW_KEY_U);
    MAS_CONVERT_INPUT(flag, KeyCode::V, GLFW_KEY_V);
    MAS_CONVERT_INPUT(flag, KeyCode::W, GLFW_KEY_W);
    MAS_CONVERT_INPUT(flag, KeyCode::X, GLFW_KEY_X);
    MAS_CONVERT_INPUT(flag, KeyCode::Y, GLFW_KEY_Y);
    MAS_CONVERT_INPUT(flag, KeyCode::Z, GLFW_KEY_Z);
    MAS_CONVERT_INPUT(flag, KeyCode::LControl, GLFW_KEY_LEFT_CONTROL);
    MAS_CONVERT_INPUT(flag, KeyCode::LShift, GLFW_KEY_LEFT_SHIFT);
    MAS_CONVERT_INPUT(flag, KeyCode::Space, GLFW_KEY_SPACE);
    MAS_CONVERT_INPUT(flag, KeyCode::Esc, GLFW_KEY_ESCAPE);
}

bool MouseInput::pressed_internal(const MouseButton input, const MouseButton compare)
{
    if ((input & compare) != MouseButton::None)
        return true;
    return false;
}

bool MouseInput::pressed(const MouseButton input) const
{
    return pressed_internal(input, flag);
}

bool MouseInput::just_pressed(const MouseButton input) const
{
    return pressed_internal(input, prev_flag);
}

std::pair<f64, f64> MouseInput::get_mouse_pos() const
{
    return std::make_pair(pos_x, pos_y);
}

std::pair<f64, f64> MouseInput::get_mouse_delta() const
{
    return std::make_pair(pos_x - prev_pos_x, pos_y - prev_pos_y);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void MouseInput::convert_glfw_input(GLFWwindow* window)
{
    prev_flag = flag;
    prev_pos_x = pos_x;
    prev_pos_y = pos_y;
    {
        glfwGetCursorPos(window, &pos_x, &pos_y);
    }
    MAS_CONVERT_INPUT(flag, MouseButton::Left, GLFW_MOUSE_BUTTON_LEFT);
    MAS_CONVERT_INPUT(flag, MouseButton::Right, GLFW_MOUSE_BUTTON_RIGHT);
    MAS_CONVERT_INPUT(flag, MouseButton::Middle, GLFW_MOUSE_BUTTON_MIDDLE);
    MAS_CONVERT_INPUT(flag, MouseButton::MouseButton4, GLFW_MOUSE_BUTTON_4);
    MAS_CONVERT_INPUT(flag, MouseButton::MouseButton5, GLFW_MOUSE_BUTTON_5);
}

InputModule::InputModule(flecs::world& world)
{
    if (const auto result = world.module<InputModule>(); !result)
        throw std::runtime_error("Failed to add input module!");

    world.import<WindowModule>();

    world.system<KeyboardInput, MouseInput, Window>("Input handling system")
        .term_at(1).singleton().term_at(2).singleton().term_at(3).singleton()
        .kind(flecs::OnLoad)
        .each([](KeyboardInput& ki, MouseInput& mi, const Window& window)
              {
                  ki.convert_glfw_input(window.get_raw_window());
                  mi.convert_glfw_input(window.get_raw_window());
              });
}
}
