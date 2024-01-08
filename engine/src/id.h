#pragma once
#include "primitives.h"

#include <type_traits>
#include <concepts>

namespace mas::id
{
using IdType = usize;

namespace details
{
struct IdBase; // Forward declaration
constexpr u32 generation_bits{ 16 };
constexpr u32 index_bits(sizeof(IdType) * 8 - generation_bits);
constexpr IdType index_mask{ (IdType{ 1 } << index_bits) - 1 };
constexpr IdType generation_mask{ ((IdType{ 1 } << generation_bits) - 1) << index_bits };
}

template <typename T>
concept Identifier = std::derived_from<T, details::IdBase> || std::same_as<T, IdType>;
constexpr IdType invalid_id{ static_cast<IdType>(-1) };
constexpr u32 min_deleted_elements{ 1024 };

using generation_type = std::conditional_t<details::generation_bits <= 16, std::conditional_t<details::generation_bits <= 8, u8, u16>, u32>;

static_assert(sizeof(generation_type) * 8 >= details::generation_bits, "Generation type does not fit the specified generation bits.");
static_assert((sizeof(IdType) - sizeof(generation_type)) > 0, "No bits left for index.");

[[nodiscard]] constexpr bool is_valid(const IdType id) noexcept
{
    return id != invalid_id;
}

[[nodiscard]] constexpr IdType index(const IdType id) noexcept
{
    return id & details::index_mask;
}

[[nodiscard]] constexpr IdType generation(const IdType id) noexcept
{
    return (id & details::generation_mask) >> details::index_bits;
}

template <typename IdType>
    requires Identifier<IdType>
[[nodiscard]] constexpr IdType new_generation(const IdType id) noexcept
{
    IdType gen{ (generation(id)) + 1 };
    if (gen == static_cast<generation_type>(-1))
    {
        gen = IdType{ generation(id) + 2 }; // To avoid getting an invalid_id when we should not.
    }
    return IdType{ (gen << details::index_bits) + index(id) };
}

namespace details
{
struct IdBase
{
    constexpr explicit IdBase(const IdType id) : id_{ id } {}

    // ReSharper disable once CppNonExplicitConversionOperator
    // We will be converting these to id_type often.
    constexpr operator IdType() const { return id_; }

private:
    IdType id_;
};
}
}

#define DEFINE_TYPED_ID(name)								                                    \
	struct name##Id final : mas::id::details::IdBase				                            \
	{														                                    \
		constexpr explicit name##Id(const mas::id::IdType id)		                            \
			: mas::id::details::IdBase {id} {}                                                  \
		constexpr name##Id() : mas::id::details::IdBase{ mas::id::invalid_id } {}	            \
	};