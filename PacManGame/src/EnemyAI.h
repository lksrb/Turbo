#pragma once

#include "Grid.h"

#include <list>
#include <random>

namespace PacMan::AI
{
    using namespace Turbo;

    namespace
    {
        template<typename T>
        static T Random(T min, T max)
        {
            static_assert(false);
        }

        static i32 Random(i32 min, i32 max)
        {
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<i32> distribution(min, max);

            return distribution(rng);
        }

        static f32 Random(f32 min, f32 max)
        {
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_real_distribution<f32> distribution(min, max);

            return distribution(rng);
        }
    }

    static std::list<Entity> RandomPath(Entity start)
    {
        std::vector<Entity> fullPath;
        std::list<Entity> randomPath;

        Entity tested = start;
        // Available directions
        std::vector<Direction> dirAvailable;

        for (auto& neighbour : start.GetComponent<TileComponent>().Neighbours)
        {
            Entity e = neighbour.Entity;
            if (e.GetComponent<TileComponent>().Type == TileType::Wall)
                continue;

            auto& startPos = start.Transform().Translation;
            auto& neightbourPos = e.Transform().Translation;

            if (neightbourPos.y > startPos.y)// UP
            {
                dirAvailable.push_back(Direction::Up);
            }
            else if (neightbourPos.y < startPos.y) // DOWN
            {
                dirAvailable.push_back(Direction::Down);
            }
            else if (neightbourPos.x > startPos.x) // RIGHT
            {
                dirAvailable.push_back(Direction::Right);
            }
            else if (neightbourPos.x < startPos.x) // LEFT
            {
                dirAvailable.push_back(Direction::Left);
            }
        }

        if (dirAvailable.size() == 0)
            return {};

        i32 random = Random(0, (i32)(dirAvailable.size() - 1));

        auto& randomDirection = dirAvailable[random];

        while (tested.GetComponent<TileComponent>().Type != TileType::Wall)
        {
            auto& testedPos = tested.Transform().Translation;

            for (auto& neighbour : tested.GetComponent<TileComponent>().Neighbours)
            {
                Entity e = neighbour.Entity;
                auto& neightbourPos = e.Transform().Translation;

                if (randomDirection == Direction::Up && neightbourPos.y > testedPos.y)// UP
                {
                    fullPath.push_back(e);
                    tested = e;
                    break;
                }
                else if (randomDirection == Direction::Down && neightbourPos.y < testedPos.y) // DOWN
                {
                    fullPath.push_back(e);
                    tested = e;
                    break;
                }
                else if (randomDirection == Direction::Right && neightbourPos.x > testedPos.x) // RIGHT
                {
                    fullPath.push_back(e);
                    tested = e;
                    break;
                }
                else if (randomDirection == Direction::Left && neightbourPos.x < testedPos.x) // LEFT
                {
                    fullPath.push_back(e);
                    tested = e;
                    break;
                }
            }
        }

        if (fullPath.empty())
        {
            return {};
        }

        i32 randomLength = Random(1, static_cast<i32>(fullPath.size() - 1));

        for (i32 i = 0; i < randomLength; ++i)
        {
            randomPath.push_back(fullPath[i]);
        }

        return randomPath;
    }
}
