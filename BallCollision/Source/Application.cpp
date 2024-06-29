#include "Application.h"

#include "MiddleAverageFilter.h"

#include <format>  // Convenient text formatting via std::format (C++20 and onwards)
#include <random>  // Random number generator to replace old srand(time(NULL))

namespace BallCollision
{

Application::Application(const std::string_view appName, const uint32_t windowSizeX, const uint32_t windowSizeY)
    : m_Window(sf::RenderWindow(sf::VideoMode(windowSizeX, windowSizeY), appName.data())), m_WindowSizeX(windowSizeX),
      m_WindowSizeY(windowSizeY)
{
    assert(!appName.empty());
}

void Application::Run()
{
    GenerateBalls();

    sf::Clock clock;
    float lastime = clock.restart().asSeconds();

    Math::MiddleAverageFilter<float, 100> fpsCounter = {};
    while (m_Window.isOpen())
    {
        PollInput();

        const float current_time = clock.getElapsedTime().asSeconds();
        const float deltaTime    = current_time - lastime;

        fpsCounter.Push(1.0f / (deltaTime));
        lastime = current_time;

        // TODO: PLACE COLLISION CODE HERE
        // Объекты создаются в случайном месте на плоскости со случайным вектором скорости, имеют радиус R
        // Объекты движутся кинетически. Пространство ограниченно границами окна
        // Напишите обработчик столкновений шаров между собой и краями окна. Как это сделать эффективно?
        // Массы пропорцианальны площадям кругов, описывающих объекты
        // Как можно было-бы улучшить текущую архитектуру кода?
        // Данный код является макетом, вы можете его модифицировать по своему усмотрению

        for (auto& ball : m_Balls)
            MoveBall(ball, deltaTime);

        auto quadTree = BuildQuadTree();
        HandleCollision(deltaTime);

        m_Window.clear();
        for (const auto& ball : m_Balls)
            DrawBall(ball);

        DrawFps(fpsCounter.CalculateAverage());

        quadTree->Show(m_Window);

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
            }
        }
    }
}

void Application::DrawBall(const Ball& ball)
{
    sf::CircleShape cirle(ball.m_Radius);
    cirle.setPosition(ball.m_Position);
    cirle.setOrigin({ball.m_Radius, ball.m_Radius});
    //  cirle.setFillColor()

    m_Window.draw(cirle);
}

void Application::MoveBall(Ball& ball, const float deltaTime)
{
    const float dX = ball.m_Direction.x * ball.m_Speed * deltaTime;
    ball.m_Position.x += dX;

    const float dY = ball.m_Direction.y * ball.m_Speed * deltaTime;
    ball.m_Position.y += dY;

    ball.UpdateBounds();
}

void Application::HandleCollision(const float deltaTime)
{
    const auto windowSizeX = static_cast<float>(m_WindowSizeX);
    const auto windowSizeY = static_cast<float>(m_WindowSizeY);
    for (size_t i{}; i < m_Balls.size(); ++i)
    {
        auto& first = m_Balls.at(i);

        for (size_t k{}; k < m_Balls.size(); ++k)
        {
            // Self-collision skip.
            if (i == k) continue;

            auto& second               = m_Balls.at(k);
            const auto collisionResult = first.IsColliding(second);
            if (!collisionResult.has_value()) continue;

            const auto& normal = collisionResult.value().m_Normal;
            const auto force   = collisionResult.value().m_Force;

            first.m_Direction  = -normal;
            second.m_Direction = normal;

            first.m_Position += -normal * second.m_Mass * force / 2.f * deltaTime;
            second.m_Position += normal * first.m_Mass * force / 2.f * deltaTime;
        }

        if (first.m_Position.x - first.m_Radius <= 0.f || first.m_Position.x + first.m_Radius >= windowSizeX)
        {
            first.m_Direction.x = -first.m_Direction.x;
            first.m_Position.x  = std::max(first.m_Radius, std::min(first.m_Position.x, windowSizeX - first.m_Radius));
        }

        if (first.m_Position.y - first.m_Radius <= 0.f || first.m_Position.y + first.m_Radius >= windowSizeY)
        {
            first.m_Direction.y = -first.m_Direction.y;
            first.m_Position.y  = std::max(first.m_Radius, std::min(first.m_Position.y, windowSizeY - first.m_Radius));
        }
    }
}

void Application::DrawFps(const float fps)
{
    const auto formattedTitle = std::format("FPS: {:.2f}", fps);
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

        const auto direction = Normalize(sf::Vector2f{(-5 + (distribution(generator) % 10)) / 3.f,  //
                                                      (-5 + (distribution(generator) % 10)) / 3.f});

        const float radius = static_cast<float>(10 + distribution(generator) % 5);
        const float speed  = static_cast<float>(30 + distribution(generator) % 30);
        m_Balls.emplace_back(position, direction, radius, speed);
    }
}

void Application::Shutdown()
{
    m_Window.close();
    m_Balls.clear();
    m_QuadTree->Clear();
}

std::unique_ptr<QuadTree> Application::BuildQuadTree()
{
    const auto windowSizeX         = static_cast<float>(m_WindowSizeX);
    const auto windowSizeY         = static_cast<float>(m_WindowSizeY);
    std::unique_ptr<QuadTree> root = std::make_unique<QuadTree>(0, sf::FloatRect{{0, 0}, {windowSizeX, windowSizeY}}, nullptr);

    for (auto& ball : m_Balls)
        root->Insert(&ball);

    return root;
}

}  // namespace BallCollision