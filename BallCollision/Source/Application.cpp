#include "Application.h"

#include "MiddleAverageFilter.h"

#include <format>  // Convenient text formatting via std::format (C++20 and onwards)
#include <random>  // Random number generator to replace old srand(time(NULL))

namespace BallCollision
{

Application::Application(const std::string_view appName, const uint32_t windowSizeX, const uint32_t windowSizeY)
    : m_Window(sf::RenderWindow(sf::VideoMode(windowSizeX, windowSizeY), appName.data())), m_WindowSizeX(windowSizeX),
      m_WindowSizeY(windowSizeY), m_AppName(appName)
{
    assert(!appName.empty());

    m_CollisionSystem =
        std::make_unique<CollisionSystem>(sf::Vector2f{static_cast<float>(m_WindowSizeX), static_cast<float>(m_WindowSizeY)});
}

void Application::Run()
{
    GenerateBalls();

    sf::Clock clock;
    float lastTime = clock.restart().asSeconds();

    Math::MiddleAverageFilter<float, 100> fpsCounter = {};
    while (m_Window.isOpen())
    {
        PollInput();

        const float current_time = clock.getElapsedTime().asSeconds();
        const float deltaTime    = current_time - lastTime;

        fpsCounter.Push(1.0f / (deltaTime));
        lastTime = current_time;

        for (auto& ball : m_Balls)
            ball.Move(deltaTime);

        sf::Clock eventTimer;

        // Build Quad Tree.
        const float treeBuildBegin = eventTimer.restart().asSeconds();
        m_CollisionSystem->BuildAccelerationStructure(m_Balls);
        const float treeBuildEnd = eventTimer.getElapsedTime().asSeconds();

        const float collisionSolvingBegin = eventTimer.restart().asSeconds();
        m_CollisionSystem->SolveCollisions(m_Balls);
        const float collisionSolvingEnd = eventTimer.getElapsedTime().asSeconds();

        m_Window.clear();
        for (const auto& ball : m_Balls)
            DrawBall(ball);

        DrawTimers(fpsCounter.CalculateAverage(), treeBuildEnd - treeBuildBegin, collisionSolvingEnd - collisionSolvingBegin);
        if (m_bDrawCollisionTree) m_CollisionSystem->DrawDebugColliders(m_Window);

        m_Window.display();
    }
}

void Application::PollInput()
{
    sf::Event event = {};
    while (m_Window.pollEvent(event))
    {
        switch (event.type)
        {
            case sf::Event::Closed:
            {
                m_Window.close();
                break;
            }
            case sf::Event::Resized:
            {
                m_WindowSizeX = event.size.width, m_WindowSizeY = event.size.height;

                // Handle window resize(to remove "stretched" ball shapes)
                sf::FloatRect visibleArea(0.f, 0.f, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
                m_Window.setView(sf::View(visibleArea));

                m_CollisionSystem->ResizeCollisionTree(sf::Vector2f{visibleArea.width, visibleArea.height});
            }
        }
    }
}

void Application::DrawBall(const Ball& ball)
{
    sf::CircleShape cirle(ball.m_Radius);
    cirle.setPosition(ball.m_Position);
    cirle.setOrigin({ball.m_Radius, ball.m_Radius});

    // Debugging AABB.
#if 0
    sf::RectangleShape rect;
    rect.setOutlineThickness(2);
    rect.setOutlineColor(sf::Color::Blue);
    rect.setFillColor(sf::Color::Transparent);
    rect.setPosition(ball.m_Bounds.left, ball.m_Bounds.top);
    rect.setSize(sf::Vector2f(ball.m_Bounds.width, ball.m_Bounds.height));
    m_Window.draw(rect);
#endif

    m_Window.draw(cirle);
}

void Application::DrawTimers(const float fps, const float treeBuildTime, const float collisionSolvingTime)
{
    const auto formattedTitle =
        std::format("{}, Objects: {}, FPS: {:.2f}, QuadTree Build Time: {:.9f} seconds, Collision Solve Time: {:.9f} seconds", m_AppName,
                    m_Balls.size(), fps, treeBuildTime, collisionSolvingTime);
    m_Window.setTitle(formattedTitle);
}

void Application::GenerateBalls()
{
    assert(m_MaxBallCount > m_MinBallCount);

    std::mt19937 generator(std::random_device{}());  // Seeding generator
    std::uniform_int_distribution<int32_t> distribution(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());

    // Randomly initialize balls
    const auto ballSpawnCount = static_cast<uint32_t>(distribution(generator) % (m_MaxBallCount - m_MinBallCount) + m_MinBallCount);
    for (uint32_t i{}; i < ballSpawnCount; ++i)
    {
        const auto position = sf::Vector2f{static_cast<float>(distribution(generator) % m_WindowSizeX),  //
                                           static_cast<float>(distribution(generator) % m_WindowSizeY)};

        sf::Vector2f direction = sf::Vector2f{(-5 + distribution(generator) * 10) / 3.f,  //
                                              (-5 + distribution(generator) * 10) / 3.f};
        const float magnitude  = std::sqrt(DotProduct(direction, direction));
        direction /= magnitude;

        const float speed  = 30 + distribution(generator) % 30;
        const float radius = 10 + distribution(generator) % 5;

        const sf::Vector2f velocity = direction * speed;
        m_Balls.emplace_back(position, velocity, radius);
    }
}

void Application::Shutdown()
{
    m_Balls.clear();
    m_CollisionSystem.reset();
}

}  // namespace BallCollision