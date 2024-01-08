#pragma once
#include "common.h"

#include "flecs/flecs.h"

#include <utility>

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

namespace mas
{
enum class KeyCode : u64
{
    None = 0b0000000000000000000000000000000000000000000000000000000000000000,
    A = 0b0000000000000000000000000000000000000000000000000000000000000010,
    B = 0b0000000000000000000000000000000000000000000000000000000000000100,
    C = 0b0000000000000000000000000000000000000000000000000000000000001000,
    D = 0b0000000000000000000000000000000000000000000000000000000000010000,
    E = 0b0000000000000000000000000000000000000000000000000000000000100000,
    F = 0b0000000000000000000000000000000000000000000000000000000001000000,
    G = 0b0000000000000000000000000000000000000000000000000000000010000000,
    H = 0b0000000000000000000000000000000000000000000000000000000100000000,
    I = 0b0000000000000000000000000000000000000000000000000000001000000000,
    J = 0b0000000000000000000000000000000000000000000000000000010000000000,
    K = 0b0000000000000000000000000000000000000000000000000000100000000000,
    L = 0b0000000000000000000000000000000000000000000000000001000000000000,
    M = 0b0000000000000000000000000000000000000000000000000010000000000000,
    N = 0b0000000000000000000000000000000000000000000000000100000000000000,
    O = 0b0000000000000000000000000000000000000000000000001000000000000000,
    P = 0b0000000000000000000000000000000000000000000000010000000000000000,
    Q = 0b0000000000000000000000000000000000000000000000100000000000000000,
    R = 0b0000000000000000000000000000000000000000000001000000000000000000,
    S = 0b0000000000000000000000000000000000000000000010000000000000000000,
    T = 0b0000000000000000000000000000000000000000000100000000000000000000,
    U = 0b0000000000000000000000000000000000000000001000000000000000000000,
    V = 0b0000000000000000000000000000000000000000010000000000000000000000,
    W = 0b0000000000000000000000000000000000000000100000000000000000000000,
    X = 0b0000000000000000000000000000000000000001000000000000000000000000,
    Y = 0b0000000000000000000000000000000000000010000000000000000000000000,
    Z = 0b0000000000000000000000000000000000000100000000000000000000000000,
    Space = 0b0000000000000000000000000000000000001000000000000000000000000000,
    LShift = 0b0000000000000000000000000000000000010000000000000000000000000000,
    LControl = 0b0000000000000000000000000000000000100000000000000000000000000000,
    Esc = 0b0000000000000000000000000000000001000000000000000000000000000000,
};

inline KeyCode operator~(KeyCode a)
{
    return static_cast<KeyCode>(~static_cast<std::underlying_type_t<KeyCode>>(a));
}

inline KeyCode operator&(KeyCode a, KeyCode b)
{
    return static_cast<KeyCode>(static_cast<std::underlying_type_t<KeyCode>>(a) & static_cast<std::underlying_type_t<KeyCode>>(b));
}

inline KeyCode& operator&=(KeyCode& a, const KeyCode b)
{
    return a = a & b;
}

inline KeyCode operator|(KeyCode a, KeyCode b)
{
    return static_cast<KeyCode>(static_cast<std::underlying_type_t<KeyCode>>(a) | static_cast<std::underlying_type_t<KeyCode>>(b));
}

inline KeyCode& operator|=(KeyCode& a, const KeyCode b)
{
    return a = a | b;
}

inline KeyCode operator-(KeyCode a, KeyCode b)
{
    return static_cast<KeyCode>(static_cast<std::underlying_type_t<KeyCode>>(a) - static_cast<std::underlying_type_t<KeyCode>>(b));
}

inline KeyCode& operator-=(KeyCode& a, const KeyCode b)
{
    return a = a - b;
}

enum class MouseButton : u64
{
    None = 0b0000000000000000000000000000000000000000000000000000000000000000,
    Left = 0b0000000000000000000000000000000000000000000000000000000000000010,
    Right = 0b0000000000000000000000000000000000000000000000000000000000000100,
    Middle = 0b0000000000000000000000000000000000000000000000000000000000001000,
    MouseButton4 = 0b0000000000000000000000000000000000000000000000000000000000010000,
    MouseButton5 = 0b0000000000000000000000000000000000000000000000000000000000100000,
};

inline MouseButton operator~(MouseButton a)
{
    return static_cast<MouseButton>(~static_cast<std::underlying_type_t<MouseButton>>(a));
}

inline MouseButton operator&(MouseButton a, MouseButton b)
{
    return static_cast<MouseButton>(static_cast<std::underlying_type_t<MouseButton>>(a) & static_cast<std::underlying_type_t<MouseButton>>(b));
}

inline MouseButton& operator&=(MouseButton& a, const MouseButton b)
{
    return a = a & b;
}

inline MouseButton operator|(MouseButton a, MouseButton b)
{
    return static_cast<MouseButton>(static_cast<std::underlying_type_t<MouseButton>>(a) | static_cast<std::underlying_type_t<MouseButton>>(b));
}

inline MouseButton& operator|=(MouseButton& a, const MouseButton b)
{
    return a = a | b;
}

inline MouseButton operator-(MouseButton a, MouseButton b)
{
    return static_cast<MouseButton>(static_cast<std::underlying_type_t<MouseButton>>(a) - static_cast<std::underlying_type_t<MouseButton>>(b));
}

inline MouseButton& operator-=(MouseButton& a, const MouseButton b)
{
    return a = a - b;
}

class KeyboardInput
{
    KeyCode flag{ KeyCode::None };
    KeyCode prev_flag{ KeyCode::None };

    static [[nodiscard]] bool pressed_internal(const KeyCode input, const KeyCode compare);

    void convert_glfw_input(GLFWwindow*);

    friend struct InputModule;

public:
    KeyboardInput() = default;
    ~KeyboardInput() = default;
    KeyboardInput(KeyboardInput&&) = default;
    KeyboardInput& operator=(KeyboardInput&&) = default;
    DISABLE_COPY(KeyboardInput)

    [[nodiscard]] bool pressed(const KeyCode input) const;

    [[nodiscard]] bool just_pressed(const KeyCode input) const;
};

class MouseInput
{
    MouseButton flag{ MouseButton::None };
    MouseButton prev_flag{ MouseButton::None };
    f64 pos_x{ 0.0 }, pos_y{ 0.0 };
    f64 prev_pos_x{ 0.0 }, prev_pos_y{ 0.0 };

    static [[nodiscard]] bool pressed_internal(const MouseButton input, const MouseButton compare);

    void convert_glfw_input(GLFWwindow*);

    friend struct InputModule;

public:
    MouseInput() = default;
    ~MouseInput() = default;
    MouseInput(MouseInput&&) = default;
    MouseInput& operator=(MouseInput&&) = default;
    DISABLE_COPY(MouseInput)

    [[nodiscard]] bool pressed(const MouseButton input) const;

    [[nodiscard]] bool just_pressed(const MouseButton input) const;

    [[nodiscard]] std::pair<f64, f64> get_mouse_pos() const;

    [[nodiscard]] std::pair<f64, f64> get_mouse_delta() const;
};

struct InputModule
{
    // ReSharper disable once CppNonExplicitConvertingConstructor
    InputModule(flecs::world& world);
};
}
