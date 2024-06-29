#pragma once

#include <SFML/Graphics.hpp> // TODO: Fix include ebanij
#include "Core.h"
#include "Ball.h"

// TODO: Put into collision solver
#include "QuadTree.h"

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

  private:
    // NOTE: Apparently, SFML has coordinate system like so Y points down and X right.
    sf::RenderWindow m_Window = {};
    uint32_t m_WindowSizeX    = {};
    uint32_t m_WindowSizeY    = {};
    uint32_t m_MinBallCount   = {};
    uint32_t m_MaxBallCount   = {};

    std::unique_ptr<QuadTree> m_QuadTree = nullptr;
    std::vector<Ball> m_Balls;

    std::unique_ptr<QuadTree> BuildQuadTree();

    void PollInput();

    void DrawBall(const Ball& ball);
    void MoveBall(Ball& ball, const float deltaTime);

    // Perfectly elastic collision between two balls
    void HandleCollision(const float deltaTime);
    void DrawFps(const float fps);

    void GenerateBalls();
    void Shutdown();
};

}  // namespace BallCollision
