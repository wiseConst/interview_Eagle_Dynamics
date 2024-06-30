#pragma once

#include "Core.h"

namespace BallCollision
{

struct Ball
{
    Ball(const sf::Vector2f& position,  //
         const sf::Vector2f& velocity,  //
         const float radius             //
         )
        : m_Position(position),           //
          m_Velocity(velocity),           //
          m_Radius(radius),               //
          m_Mass(s_PI * radius * radius)  //
    {
        UpdateBounds();
    }
    Ball()  = default;
    ~Ball() = default;

    void UpdateBounds() { m_Bounds = {m_Position.x, m_Position.y, m_Radius, m_Radius}; }

    void Move(const float deltaTime)
    {
        m_Position += m_Velocity * deltaTime;

        UpdateBounds();
    }

    sf::Vector2f m_Position = sf::Vector2f{0.f, 0.f};
    sf::Vector2f m_Velocity = sf::Vector2f{0.f, 0.f};
    const float m_Radius    = 0.f;
    const float m_Mass      = 0.f;
    sf::FloatRect m_Bounds  = {};
};

}  // namespace BallCollision