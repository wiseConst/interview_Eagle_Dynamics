#pragma once

#include "Ball.h"
#include "Core.h"

#include <vector>
#include <memory>

// NOTE: Testing
#include <SFML/Graphics.hpp>

namespace BallCollision
{

class QuadTree final
{
  private:
    enum ESubdivisionType : uint8_t
    {
        SUBDIVISON_TYPE_NORTH_WEST = 0,  // Top left
        SUBDIVISON_TYPE_NORTH_EAST,      // Top right
        SUBDIVISON_TYPE_SOUTH_EAST,      // Bottom right
        SUBDIVISON_TYPE_SOUTH_WEST,      // Bottom left
        SUBDIVISON_TYPE_NONE = UINT8_MAX
    };

  public:
    QuadTree() = default;
    QuadTree(const uint32_t level, const sf::FloatRect& bounds, QuadTree* parent) : m_Level(level), m_Bounds(bounds), m_ParentNode(parent)
    {
    }
    ~QuadTree() = default;

    void Clear()
    {
        m_Objects.clear();

        for (uint32_t i{}; i < 4; ++i)
        {
            if (!m_Nodes[i]) continue;

            m_Nodes[i]->Clear();
            m_Nodes[i].reset();
        }
    }

    void Insert(Ball* ball)
    {
        assert(ball);

        const auto bIsLeaf = IsLeaf();
        if (!bIsLeaf)
        {
            const auto quadrantIndex = GetQuadrantIndex(ball->m_Bounds);
            if (quadrantIndex != ESubdivisionType::SUBDIVISON_TYPE_NONE)
            {
                m_Nodes[quadrantIndex]->Insert(ball);
                return;
            }
        }

        m_Objects.emplace_back(ball);
        // NOTE: Split in case threshold reached, doesn't have children(so we can spawn them), and check depth level threshold.
        if (bIsLeaf && m_Objects.size() + 1 >= s_ObjectThreshold && m_Level < s_DepthThreshold)
        {
            Subdivide();

            auto objIterator = m_Objects.begin();
            while (objIterator != m_Objects.end())
            {
                auto ball = *objIterator;
                assert(ball);

                const auto quadrantIndex = GetQuadrantIndex(ball->m_Bounds);
                if (quadrantIndex != ESubdivisionType::SUBDIVISON_TYPE_NONE)
                {
                    m_Nodes[quadrantIndex]->Insert(ball);
                    objIterator = m_Objects.erase(objIterator);
                }
                else
                    ++objIterator;
            }

            m_Objects.shrink_to_fit();
        }
    }

    NODISCARD std::vector<Ball*> QueryCollisions(const sf::FloatRect& area)
    {
        std::vector<Ball*> possibleOverlaps = {};
        QueryCollisionsInternal(possibleOverlaps, area);

        std::vector<Ball*> actualOverlaps;
        for (auto ball : possibleOverlaps)
        {
            if (!ball || !area.intersects(ball->m_Bounds)) continue;

            actualOverlaps.emplace_back(ball);
        }

        return actualOverlaps;
    }

    void QueryCollisionsInternal(std::vector<Ball*>& outOverlappingObjects, const sf::FloatRect& area)
    {
        outOverlappingObjects.insert(outOverlappingObjects.end(), m_Objects.begin(), m_Objects.end());
        if (IsLeaf()) return;

        const auto quadrantIndex = GetQuadrantIndex(area);
        if (quadrantIndex == ESubdivisionType::SUBDIVISON_TYPE_NONE)
        {
            for (uint32_t i{}; i < 4; ++i)
            {
                if (!m_Nodes[i]->GetBounds().intersects(area)) continue;

                m_Nodes[i]->QueryCollisionsInternal(outOverlappingObjects, area);
            }
        }
        else
        {
            m_Nodes[quadrantIndex]->QueryCollisionsInternal(outOverlappingObjects, area);
        }
    }

    ESubdivisionType GetQuadrantIndex(const sf::FloatRect& objectBounds)
    {
        const float verticalDividingLine   = m_Bounds.left + m_Bounds.width * 0.5f;
        const float horizontalDividingLine = m_Bounds.top + m_Bounds.height * 0.5f;

        const bool bDoesFitInNorth =
            objectBounds.top < horizontalDividingLine && (objectBounds.height + objectBounds.top < horizontalDividingLine);
        const bool bDoesFitInSouth = objectBounds.top > horizontalDividingLine;

        if (objectBounds.left > verticalDividingLine)  // east
        {
            if (bDoesFitInNorth)
                return ESubdivisionType::SUBDIVISON_TYPE_NORTH_EAST;
            else if (bDoesFitInSouth)
                return ESubdivisionType::SUBDIVISON_TYPE_SOUTH_EAST;
        }
        else if (objectBounds.left < verticalDividingLine && (objectBounds.left + objectBounds.width < verticalDividingLine))  // west
        {
            if (bDoesFitInNorth)
                return ESubdivisionType::SUBDIVISON_TYPE_NORTH_WEST;
            else if (bDoesFitInSouth)
                return ESubdivisionType::SUBDIVISON_TYPE_SOUTH_WEST;
        }

        return ESubdivisionType::SUBDIVISON_TYPE_NONE;
    }

    void Show(sf::RenderWindow& window)
    {
        sf::RectangleShape rect{};
        rect.setOutlineThickness(m_Level + .1f);
        rect.setFillColor(sf::Color::Transparent);
        // rect.setFillColor(sf::Color(0, 123, 179, 55));
        rect.setPosition(m_Bounds.left /*+ m_Bounds.width * .5f*/, m_Bounds.top /*+ m_Bounds.height * .5f*/);
        rect.setSize(sf::Vector2f(m_Bounds.width /* * .5f*/, m_Bounds.height /** .5f*/));
        window.draw(rect);

        if (IsLeaf()) return;

        for (uint32_t i{}; i < 4; ++i)
        {
            if (!m_Nodes[i]) continue;

            m_Nodes[i]->Show(window);
        }
    }

    NODISCARD FORCEINLINE const auto& GetBounds() const { return m_Bounds; }

  private:
    sf::FloatRect m_Bounds = {};

    static constexpr auto s_ObjectThreshold = std::size_t(16);  // Max number of values a node can contain before we try to split it.
    static constexpr auto s_DepthThreshold  = std::size_t(8);

    // nullptr if this is the base node.
    QuadTree* m_ParentNode = nullptr;
    std::unique_ptr<QuadTree> m_Nodes[4];

    std::vector<Ball*> m_Objects;

    // How deep the current node is from the base node.
    // The first node starts at 0 and then its child node
    // is at level 1 and so on.
    uint32_t m_Level = {};

    FORCEINLINE bool IsLeaf() const { return m_Nodes[0] == nullptr; }

    void Subdivide()
    {
        const auto childWidth  = m_Bounds.width / 2;
        const auto childHeight = m_Bounds.height / 2;

        const auto nwBounds                                   = sf::FloatRect(m_Bounds.left, m_Bounds.top, childWidth, childHeight);
        m_Nodes[ESubdivisionType::SUBDIVISON_TYPE_NORTH_WEST] = std::make_unique<QuadTree>(m_Level + 1, nwBounds, this);

        const auto neBounds = sf::FloatRect(m_Bounds.left + childWidth, m_Bounds.top, childWidth, childHeight);
        m_Nodes[ESubdivisionType::SUBDIVISON_TYPE_NORTH_EAST] = std::make_unique<QuadTree>(m_Level + 1, neBounds, this);

        const auto seBounds = sf::FloatRect(m_Bounds.left + childWidth, m_Bounds.top + childHeight, childWidth, childHeight);
        m_Nodes[ESubdivisionType::SUBDIVISON_TYPE_SOUTH_EAST] = std::make_unique<QuadTree>(m_Level + 1, seBounds, this);

        const auto swBounds = sf::FloatRect(m_Bounds.left, m_Bounds.top + childHeight, childWidth, childHeight);
        m_Nodes[ESubdivisionType::SUBDIVISON_TYPE_SOUTH_WEST] = std::make_unique<QuadTree>(m_Level + 1, swBounds, this);
    }
};

}  // namespace BallCollision