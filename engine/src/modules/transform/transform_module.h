#pragma once
#include "flecs/flecs.h"
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace mas
{
struct Transform
{
    glm::vec3 position{ 0.0f };
    glm::vec3 scale{ 1.0f };
    glm::quat rotation{};
};

//Internal type to the engine, use mas::Transform instead.
struct GlobalTransform
{
    glm::mat4 transform{ 1.0f };
};

struct TransformModule
{
    // ReSharper disable once CppNonExplicitConvertingConstructor
    TransformModule(const flecs::world& world);
};
}
