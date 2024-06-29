#pragma once

#include <array>
#include <execution>  // Threading policy
#include <numeric>    // For std::reduce

#include "Core.h"

namespace BallCollision
{

namespace Math
{

template <typename T, std::size_t size> class MiddleAverageFilter final
{
  private:
    static_assert(size > 0, "MiddleAverageFilter's size must be greater than 0!");

  public:
    MiddleAverageFilter()  = default;
    ~MiddleAverageFilter() = default;

    void Push(const T& value)
    {
        m_Data[m_ID] = value;
        m_ID         = (m_ID + 1) % size;
    }

    NODISCARD FORCEINLINE T CalculateAverage() const
    {
        const T sum = std::reduce(std::execution::par, m_Data.begin(), m_Data.end(), /* init value = */ T{});
        return sum / static_cast<T>(size);
    }

  private:
    std::array<T, size> m_Data = {};
    std::size_t m_ID           = {};
};

}  // namespace Math

}  // namespace BallCollision
