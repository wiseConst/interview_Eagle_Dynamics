#pragma once

#include "SFML/System/Vector2.hpp"
#include "Core.h"

namespace BallCollision
{

struct CollisionResult
{
    CollisionResult(const sf::Vector2f& normal, const float force) : m_Normal(normal), m_Force(force) {}
    CollisionResult()  = default;
    ~CollisionResult() = default;

    sf::Vector2f m_Normal = sf::Vector2f{0.f, 0.f};
    float m_Force         = 0.f;
};

FORCEINLINE float DotProduct(const sf::Vector2f& lhs, const sf::Vector2f& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

FORCEINLINE sf::Vector2f Normalize(const sf::Vector2f& vec)
{
    const auto magnitude = std::sqrt(DotProduct(vec, vec)) + 0.0001f;
    assert(magnitude > 0.0);
    return vec / magnitude;
}

struct Ball
{
    Ball(const sf::Vector2f& position,    //
         const sf::Vector2f& direction,   //
         const float radius,              //
         const float speed)               //
        : m_Position(position),           //
          m_Direction(direction),         //
          m_Radius(radius),               //
          m_Speed(speed),                 //
          m_Mass(M_PI * radius * radius)  //
    {
        UpdateBounds();
    }
    Ball()  = default;
    ~Ball() = default;

    // TODO: Move out ?
    FORCEINLINE std::optional<CollisionResult> IsColliding(const Ball& other) const
    {
        // Calculate squared distance between centers
        const sf::Vector2f distance = other.m_Position - m_Position;
        const float distance2       = DotProduct(distance, distance);

        // Spheres intersect if squared distance is less than squared sum of radii
        const float radiusSum = m_Radius + other.m_Radius;
        if (distance2 > radiusSum * radiusSum) return std::nullopt;

        const sf::Vector2f normal = Normalize(distance);
        const float force         = radiusSum - std::sqrt(distance2);
        return std::make_optional<CollisionResult>(normal, force);
    }

    void UpdateBounds() { m_Bounds = {m_Position.x, m_Position.y, m_Radius, m_Radius}; }

    sf::Vector2f m_Position  = sf::Vector2f{0.f, 0.f};
    sf::Vector2f m_Direction = sf::Vector2f{0.f, 0.f};
    float m_Radius           = 0.f;
    float m_Speed            = 0.f;
    float m_Mass             = 0.f;
    sf::FloatRect m_Bounds   = {};
};

}  // namespace BallCollision