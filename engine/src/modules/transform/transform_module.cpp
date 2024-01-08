#include "transform_module.h"

#include <stdexcept>

namespace mas
{
TransformModule::TransformModule(const flecs::world& world)
{
    if (const auto result = world.module<TransformModule>(); !result)
        throw std::runtime_error("Failed to add transform module");

    world.system<const Transform, GlobalTransform>("Transform system")
        .kind(flecs::OnValidate)
        .multi_threaded(true)
        .each([](const Transform& t, GlobalTransform& global_t)
              {
                  global_t.transform = glm::translate(glm::mat4(1.0f), t.position) * glm::toMat4(glm::quat(t.rotation)) * glm::scale(glm::mat4(1.0f), t.scale);
              });
}
}
