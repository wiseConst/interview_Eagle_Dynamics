#pragma once

#include <SFML/Graphics.hpp>
#include "Core.h"
#include "Ball.h"
#include "CollisionSystem.h"

namespace BallCollision
{

class Application final
{
  public:
    Application(const std::string_view appName, const uint32_t windowSizeX, const uint32_t windowSizeY);
    ~Application() { Shutdown(); }

    void Run();

    void SetMinBallCount(const uint32_t minBallCount) { m_MinBallCount = minBallCount; }
    void SetMaxBallCount(const uint32_t maxBallCount) { m_MaxBallCount = maxBallCount; }
    void SetFrameRateLimit(const uint32_t limit) { m_Window.setFramerateLimit(limit); }
    void SetDrawCollisionTree(const bool bDrawCollisionTree) { m_bDrawCollisionTree = bDrawCollisionTree; }

  private:
    sf::RenderWindow m_Window = {};
    uint32_t m_WindowSizeX    = {};
    uint32_t m_WindowSizeY    = {};
    uint32_t m_MinBallCount   = {};
    uint32_t m_MaxBallCount   = {};
    bool m_bDrawCollisionTree = false;

    std::string m_AppName = {};

    std::unique_ptr<CollisionSystem> m_CollisionSystem = nullptr;
    std::vector<Ball> m_Balls;

    void PollInput();

    void DrawBall(const Ball& ball);
    void DrawTimers(const float fps, const float treeBuildTime, const float collisionSolvingTime);

    void GenerateBalls();
    void Shutdown();
};

}  // namespace BallCollision
