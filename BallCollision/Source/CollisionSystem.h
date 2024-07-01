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
    CollisionSystem(const sf::Vector2f& screenBounds) noexcept;
    ~CollisionSystem();

    FORCEINLINE void DrawDebugColliders(sf::RenderWindow& window) { m_CollisionTree->Show(window); }

    FORCEINLINE void ResizeCollisionTree(const sf::Vector2f& screenBounds)
    {
        m_CollisionTree->Clear();
        m_CollisionTree = std::make_unique<QuadTree<8, 8>>(0, sf::FloatRect{{0, 0}, {screenBounds.x, screenBounds.y}}, nullptr);
    }

    void BuildAccelerationStructure(std::vector<Ball>& balls);
    void SolveCollisions(std::vector<Ball>& balls);

  private:
    std::unique_ptr<QuadTree<8, 8>> m_CollisionTree = nullptr;

    FORCEINLINE std::optional<CollisionResult> AreBallsColliding(const Ball& lhs, const Ball& rhs) const;
};

}  // namespace BallCollision