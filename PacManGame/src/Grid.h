#pragma once

#include <Turbo/Scene/Entity.h>

#include "GameEvent.h"
#include "Player.h"

#include <string>
#include <vector>

class Turbo::Scene;

namespace PacMan
{
    using namespace Turbo;

    enum class TileType : u32
    {
        Empty = 0,
        Wall,
    };

    struct TileComponent
    {
        TileType Type;
        
        // Navigation
        u32 GridX;
        u32 GridY;

        Entity LootEntity;

        struct Neighbour
        {
            Entity Entity;
            Direction Direction;
        };
        std::vector<Neighbour> Neighbours;
    };

    struct LootComponent
    {
        bool Looted = false;
        u32 Value = 1;
    };

    class Grid
    {
    public:
        struct Config
        {
            u32 Height;
            u32 Width;
            Scene* CurrentScene;
            std::string MapSkeleton;
        };

        Grid(const Config& config);
        ~Grid();

        void Generate();
        void SetGameEventCallback(const GameEventCallback& callback) { m_DispatchEventCallback = callback; }

        const std::vector<Entity>& GetGridEntities() const { return m_GridEntities; }
        const std::vector<Entity>& GetInteractables() const { return m_WallEntities; }
    private:
        void GenerateTile(Entity e, char tile);
    private:
        GameEventCallback m_DispatchEventCallback;

        std::vector<Entity> m_WallEntities;
        std::vector<Entity> m_GridEntities;

        Grid::Config m_Config;
    };
}
