#include "Application.h"

static constexpr uint32_t s_MinBallCount = 300;
static constexpr uint32_t s_MaxBallCount = 900;

int32_t main()
{
    auto ballCollisionDemo = std::make_unique<BallCollision::Application>("Fast 2D Circle Collision System", 1024, 768);
     ballCollisionDemo->SetFrameRateLimit(60);

    ballCollisionDemo->SetMinBallCount(s_MinBallCount);
    ballCollisionDemo->SetMaxBallCount(s_MaxBallCount);
  //  ballCollisionDemo->SetDrawCollisionTree(true);

    ballCollisionDemo->Run();

    return 0;
}
