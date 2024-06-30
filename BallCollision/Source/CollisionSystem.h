#pragma once

#include "Core.h"
#include "Ball.h"

#include "QuadTree.h"

namespace BallCollision
{

struct CollisionResult
{
    CollisionResult(const sf::Vector2f& normal, const float overlapLength) : m_Normal(normal), m_OverlapLength(overlapLength) {}
    CollisionResult()  = default;
    ~CollisionResult() = default;

    sf::Vector2f m_Normal = sf::Vector2f{0.f, 0.f};
    float m_OverlapLength = 0.f;
};

class CollisionSystem final
{
  public:
    CollisionSystem(const sf::Vector2f& screenBounds)
    {
        m_CollisionTree = std::make_unique<QuadTree<8, 8>>(0, sf::FloatRect{{0, 0}, {screenBounds.x, screenBounds.y}}, nullptr);
    }
    ~CollisionSystem() { m_CollisionTree.reset(); }

    void DrawDebugColliders(sf::RenderWindow& window) { m_CollisionTree->Show(window); }

    void ResizeCollisionTree(const sf::Vector2f& screenBounds)
    {
        m_CollisionTree->Clear();
        m_CollisionTree = std::make_unique<QuadTree<8, 8>>(0, sf::FloatRect{{0, 0}, {screenBounds.x, screenBounds.y}}, nullptr);
    }

    void BuildAccelerationStructure(std::vector<Ball>& balls)
    {
        assert(!balls.empty());
        m_CachedCollidingBalls.clear();

        // NOTE: Rebuilding every frame.
        ResizeCollisionTree({m_CollisionTree->GetBounds().width, m_CollisionTree->GetBounds().height});

        for (auto& ball : balls)
            m_CollisionTree->Insert(&ball);
    }

    void SolveScreenBoundsCollision(std::vector<Ball>& balls)
    {
        for (auto& ball : balls)
        {
            if (ball.m_Position.x - ball.m_Radius <= 0.f || ball.m_Position.x + ball.m_Radius >= m_CollisionTree->GetBounds().width)
            {
                ball.m_Velocity.x = -ball.m_Velocity.x;
                ball.m_Position.x =
                    std::max(ball.m_Radius, std::min(ball.m_Position.x, m_CollisionTree->GetBounds().width - ball.m_Radius));

                ball.UpdateBounds();
            }

            if (ball.m_Position.y - ball.m_Radius <= 0.f || ball.m_Position.y + ball.m_Radius >= m_CollisionTree->GetBounds().height)
            {
                ball.m_Velocity.y = -ball.m_Velocity.y;
                ball.m_Position.y =
                    std::max(ball.m_Radius, std::min(ball.m_Position.y, m_CollisionTree->GetBounds().height - ball.m_Radius));

                ball.UpdateBounds();
            }
        }
    }

    void SolveStaticCollisions(std::vector<Ball>& balls)
    {
        assert(m_CachedCollidingBalls.empty());

        for (auto& ball : balls)
        {
            auto overlappedBalls = m_CollisionTree->QueryCollisions(ball.m_Bounds);

            for (Ball* overlappedBall : overlappedBalls)
            {
                if (!overlappedBall || &ball == overlappedBall) continue;

                const auto collisionResult = AreBallsColliding(ball, *overlappedBall);
                if (!collisionResult.has_value()) continue;

                const auto& normal       = collisionResult.value().m_Normal;
                const auto overlapLength = collisionResult.value().m_OverlapLength;

                ball.m_Position -= normal * overlapLength;
                ball.UpdateBounds();

                overlappedBall->m_Position += normal * overlapLength;
                overlappedBall->UpdateBounds();

                m_CachedCollidingBalls.emplace_back(&ball, overlappedBall);
            }
        }
    }

    void SolveDynamicCollisions()
    {
        for (auto& [ball, target] : m_CachedCollidingBalls)
        {
            if (!ball || !target || ball == target) continue;

            const float distance = std::sqrt((ball->m_Position.x - target->m_Position.x) * (ball->m_Position.x - target->m_Position.x) +
                                             (ball->m_Position.y - target->m_Position.y) * (ball->m_Position.y - target->m_Position.y)) +
                                   s_BC_KINDA_SMALL_NUMBER;

            const sf::Vector2f normal = (ball->m_Position - target->m_Position) / distance;
            const sf::Vector2f tangent{-normal.y, normal.x};

            // Apply tangential and normal responses.
            const float firstTangentSpeed  = DotProduct(ball->m_Velocity, tangent);
            const float secondTangentSpeed = DotProduct(target->m_Velocity, tangent);

            const float firstSpeed  = DotProduct(ball->m_Velocity, normal);
            const float secondSpeed = DotProduct(target->m_Velocity, normal);

            const float massSum  = ball->m_Mass + target->m_Mass;
            const float massDiff = ball->m_Mass - target->m_Mass;

            const float firstNormalSpeed  = ((2 * target->m_Mass * secondSpeed) + firstSpeed * massDiff) / massSum;
            const float secondNormalSpeed = ((2 * ball->m_Mass * firstSpeed) - secondSpeed * massDiff) / massSum;

            ball->m_Velocity   = tangent * firstTangentSpeed + normal * firstNormalSpeed;
            target->m_Velocity = tangent * secondTangentSpeed + normal * secondNormalSpeed;
        }
    }

  private:
    std::unique_ptr<QuadTree<8, 8>> m_CollisionTree             = nullptr;
    std::vector<std::pair<Ball*, Ball*>> m_CachedCollidingBalls = {};

    FORCEINLINE std::optional<CollisionResult> AreBallsColliding(const Ball& lhs, const Ball& rhs) const
    {
        // Calculate squared distance between centers
        const sf::Vector2f distanceVec = lhs.m_Position - rhs.m_Position;
        const float distance2          = DotProduct(distanceVec, distanceVec);

        // Spheres intersect if squared distance is less than squared sum of radii
        const float radiusSum = lhs.m_Radius + rhs.m_Radius;
        if (distance2 > radiusSum * radiusSum) return std::nullopt;

        const float distance      = std::sqrt(distance2) + s_BC_KINDA_SMALL_NUMBER;
        const sf::Vector2f normal = distanceVec / distance;
        const float overlapLength = 0.5f * (distance - radiusSum);
        return std::make_optional<CollisionResult>(normal, overlapLength);
    }
};

}  // namespace BallCollision