#include "Application.h"

static constexpr uint32_t s_MinBallCount = 300;
static constexpr uint32_t s_MaxBallCount = 500;

int32_t main()
{
    auto ballCollisionDemo = std::make_unique<BallCollision::Application>("Fast 2D Circle Collision Handler", 1024, 768);
    ballCollisionDemo->SetMinBallCount(s_MinBallCount);
    ballCollisionDemo->SetMaxBallCount(s_MaxBallCount);
    // ballCollisionDemo->SetFrameRateLimit(60);
    ballCollisionDemo->Run();

    return 0;
}
