#pragma once

#include "Core.h"
#include "Ball.h"

#include <array>
#include <vector>
#include <memory>

// NOTE: Only for drawing debug colliders.
#include <SFML/Graphics.hpp>

namespace BallCollision
{

// Max number of values a node can contain before we try to split it.
template <std::size_t DepthThreshold = std::size_t(8), std::size_t ObjectThreshold = std::size_t(16)> class QuadTree final
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

    NODISCARD FORCEINLINE const sf::FloatRect& GetBounds() const { return m_Bounds; }

    void Insert(Ball* ball)
    {
        assert(ball);

        // Firstly try to place in children quadrants
        const auto bIsLeaf = IsLeaf();
        if (!bIsLeaf)
        {
            const auto quadrantIndex = GetQuadrantIndex(ball->m_Bounds);
            if (quadrantIndex != ESubdivisionType::SUBDIVISON_TYPE_NONE)
            {
                m_Nodes.at(quadrantIndex)->Insert(ball);
                return;
            }
        }

        // NOTE: Split in case threshold reached, doesn't have children(so we can spawn them), and check depth level threshold.
        m_Objects.emplace_back(ball);
        if (bIsLeaf && m_Objects.size() + 1 >= ObjectThreshold && m_Level < DepthThreshold)
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
                    m_Nodes.at(quadrantIndex)->Insert(ball);
                    objIterator = m_Objects.erase(objIterator);
                }
                else
                    ++objIterator;
            }

            m_Objects.shrink_to_fit();
        }
    }

    void Clear()
    {
        m_Objects.clear();

        for (uint32_t i{}; i < m_Nodes.size(); ++i)
        {
            if (!m_Nodes.at(i)) continue;

            m_Nodes.at(i)->Clear();
            m_Nodes.at(i).reset();
        }
    }

    NODISCARD std::vector<Ball*> QueryPossibleIntersections(const sf::FloatRect& area)
    {
        std::vector<Ball*> overlappedObjects = {};
        QueryPossibleIntersectionsInternal(overlappedObjects, area);

        return overlappedObjects;
    }

    void Show(sf::RenderWindow& window) const
    {
        sf::RectangleShape rect{};
        rect.setOutlineThickness(m_Level * 0.75f);
        rect.setOutlineColor(sf::Color::Green);
        rect.setFillColor(sf::Color::Transparent);
        rect.setPosition(m_Bounds.left, m_Bounds.top);
        rect.setSize(sf::Vector2f(m_Bounds.width, m_Bounds.height));
        window.draw(rect);

        if (IsLeaf()) return;

        for (uint32_t i{}; i < 4; ++i)
        {
            if (!m_Nodes.at(i)) continue;

            m_Nodes.at(i)->Show(window);
        }
    }

  private:
    sf::FloatRect m_Bounds = {};

    // nullptr if this is the base node.
    QuadTree* m_ParentNode = nullptr;
    std::array<std::unique_ptr<QuadTree>, 4> m_Nodes;
    std::vector<Ball*> m_Objects;

    // How deep the current node is from the base node.
    // The first node starts at 0 and then its child node
    // is at level 1 and so on until depth treshold.
    uint32_t m_Level = {};

    FORCEINLINE bool IsLeaf() const { return m_Nodes.at(0) == nullptr; }

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

    ESubdivisionType GetQuadrantIndex(const sf::FloatRect& objectBounds) const
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

    void QueryPossibleIntersectionsInternal(std::vector<Ball*>& outOverlappingObjects, const sf::FloatRect& area)
    {
        // 1. Add items from current quadrant if they do overlap.
        for (Ball* ball : m_Objects)
        {
            if (!ball || !area.intersects(ball->m_Bounds)) continue;

            outOverlappingObjects.emplace_back(ball);
        }

        // Check if the inner rectangle is completely inside the outer rectangle
        const auto doesRectContain = [](const sf::FloatRect& outer, const sf::FloatRect& inner)
        {
            const bool left   = inner.left >= outer.left;
            const bool right  = (inner.left + inner.width) <= (outer.left + outer.width);
            const bool top    = inner.top >= outer.top;
            const bool bottom = (inner.top + inner.height) <= (outer.top + outer.height);

            return left && right && top && bottom;
        };

        // 2. Check children quadrants.
        for (auto& childrenQuadrant : m_Nodes)
        {
            if (!childrenQuadrant) continue;

            const auto& childrenBounds = childrenQuadrant->GetBounds();

            // If child is entirely contained within the area, no need to check the boundaries,
            // simply add all of its children recursively.
            if (doesRectContain(area, childrenBounds)) childrenQuadrant->PushChildrenObjects(outOverlappingObjects);

            // But if child overlaps with search area, additional checks need to be made.
            else if (childrenBounds.intersects(area))
                childrenQuadrant->QueryPossibleIntersectionsInternal(outOverlappingObjects, area);
        }
    }

    void PushChildrenObjects(std::vector<Ball*>& outOverlappingObjects)
    {
        for (Ball* ball : m_Objects)
        {
            if (ball) outOverlappingObjects.emplace_back(ball);
        }

        for (auto& childrenQuadrant : m_Nodes)
        {
            if (childrenQuadrant) childrenQuadrant->PushChildrenObjects(outOverlappingObjects);
        }
    }
};

}  // namespace BallCollision