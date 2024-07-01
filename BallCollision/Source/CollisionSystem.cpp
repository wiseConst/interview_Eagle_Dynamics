#include "CollisionSystem.h"

namespace BallCollision
{

CollisionSystem::CollisionSystem(const sf::Vector2f& screenBounds) noexcept
{
    m_CollisionTree = std::make_unique<QuadTree<8, 8>>(0, sf::FloatRect{{0, 0}, {screenBounds.x, screenBounds.y}}, nullptr);
}

CollisionSystem::~CollisionSystem()
{
    m_CollisionTree.reset();
}

void CollisionSystem::BuildAccelerationStructure(std::vector<Ball>& balls)
{
    assert(!balls.empty());

    // NOTE: Rebuilding every frame.
    ResizeCollisionTree({m_CollisionTree->GetBounds().width, m_CollisionTree->GetBounds().height});

    for (auto& ball : balls)
        m_CollisionTree->Insert(&ball);
}

void CollisionSystem::SolveCollisions(std::vector<Ball>& balls)
{
    // 1. Resolve static collisions, so one ball can't exist inside the other.
    std::vector<std::pair<Ball*, Ball*>> collidingBalls = {};
    for (auto& ball : balls)
    {
        auto possibleIntersections = m_CollisionTree->QueryPossibleIntersections(ball.m_Bounds);

        for (Ball* otherBall : possibleIntersections)
        {
            if (!otherBall || &ball == otherBall) continue;

            const auto collisionResult = AreBallsColliding(ball, *otherBall);
            if (!collisionResult.has_value()) continue;

            const auto& normal       = collisionResult.value().m_Normal;
            const auto overlapLength = collisionResult.value().m_OverlapLength;

            ball.m_Position -= normal * overlapLength;

            otherBall->m_Position += normal * overlapLength;
            otherBall->UpdateBounds();

            collidingBalls.emplace_back(&ball, otherBall);
        }
        ball.UpdateBounds();

        // 2. Solve screen bounds.
        if (ball.m_Position.x - ball.m_Radius <= 0.f || ball.m_Position.x + ball.m_Radius >= m_CollisionTree->GetBounds().width)
        {
            ball.m_Velocity.x = -ball.m_Velocity.x;
            ball.m_Position.x = std::max(ball.m_Radius, std::min(ball.m_Position.x, m_CollisionTree->GetBounds().width - ball.m_Radius));

            ball.UpdateBounds();
        }

        if (ball.m_Position.y - ball.m_Radius <= 0.f || ball.m_Position.y + ball.m_Radius >= m_CollisionTree->GetBounds().height)
        {
            ball.m_Velocity.y = -ball.m_Velocity.y;
            ball.m_Position.y = std::max(ball.m_Radius, std::min(ball.m_Position.y, m_CollisionTree->GetBounds().height - ball.m_Radius));

            ball.UpdateBounds();
        }
    }

    // 3. Solve an actual dynamic perfectly elastic collisions.
    for (auto& [ball, target] : collidingBalls)
    {
        if (!ball || !target || ball == target) continue;

        float distance = std::sqrt((ball->m_Position.x - target->m_Position.x) * (ball->m_Position.x - target->m_Position.x) +
                                   (ball->m_Position.y - target->m_Position.y) * (ball->m_Position.y - target->m_Position.y));
        if (distance == 0.0f) distance = s_BC_KINDA_SMALL_NUMBER;

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

std::optional<CollisionResult> CollisionSystem::AreBallsColliding(const Ball& lhs, const Ball& rhs) const
{
    // Calculate squared distance between centers
    const sf::Vector2f distanceVec = lhs.m_Position - rhs.m_Position;
    const float distance2          = DotProduct(distanceVec, distanceVec);

    // Spheres intersect if squared distance is less than squared sum of radii
    const float radiusSum = lhs.m_Radius + rhs.m_Radius;
    if (distance2 > radiusSum * radiusSum) return std::nullopt;

    float distance = std::sqrt(distance2);
    if (distance == 0.0f) distance = s_BC_KINDA_SMALL_NUMBER;

    const sf::Vector2f normal = distanceVec / distance;
    const float overlapLength = 0.5f * (distance - radiusSum);
    return std::make_optional<CollisionResult>(normal, overlapLength);
}

}  // namespace BallCollision