#pragma once

#include <cstdint>
#include <optional>
#include <cassert>
#include <memory>

#define _USE_MATH_DEFINES
#include <cmath>

#define FORCEINLINE __forceinline
#define NODISCARD [[nodiscard]]

#include "SFML/System/Vector2.hpp"

namespace BallCollision
{
// This one used for collision detection, because in highly dense area, distance between 2 balls may be(almost) zero.
static constexpr auto s_BC_KINDA_SMALL_NUMBER = 10.E-4f;
static constexpr float s_PI                   = 3.14159265358979323846f;  // pi

FORCEINLINE float DotProduct(const sf::Vector2f& lhs, const sf::Vector2f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

}  // namespace BallCollision