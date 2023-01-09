#include "Grid.h"

namespace PacMan
{
    Grid::Grid(const Config& config)
        : m_Config(config)
    {
    }

    Grid::~Grid()
    {
    }

    void Grid::Generate()
    {
        m_GridEntities.resize(m_Config.Width * m_Config.Height);

        // Each tile is an entity
        for (u32 y = 0; y < m_Config.Height; ++y)
        {
            for (u32 x = 0; x < m_Config.Width; ++x)
            {
                // Create entity on specific index
                Entity e = m_GridEntities[y * m_Config.Width + x] = m_Config.CurrentScene->CreateEntity();

                // Render every tile
                e.AddComponent<SpriteRendererComponent>();

                // Info about the tile 
                auto& tile = e.AddCustomComponent<TileComponent>();
                tile.GridX = x;
                tile.GridY = y;

                // Offset that centers the grid
                glm::vec2 offset = { m_Config.Width / 2.0f - 0.5f, m_Config.Height / 2.0f - 0.5f };

                // Set tile position

                auto& translation = e.Transform().Translation;
                translation.x = static_cast<f32>(x - offset.x);
                translation.y = static_cast<f32>(y - offset.y);

                // Generate tile according to string map skeleton
                GenerateTile(e, m_Config.MapSkeleton[y * m_Config.Width + x]);
            }
        }

        // Every tile knows its neighbour (up, down, left, right).
        for (u32 y = 0; y < m_Config.Height; ++y)
        {
            for (u32 x = 0; x < m_Config.Width; ++x)
            {
                auto& tile = m_GridEntities[y * m_Config.Width + x].GetComponent<TileComponent>();

                if (y < m_Config.Height - 1)
                    tile.Neighbours.push_back({ m_GridEntities[(y + 1) * m_Config.Width + x], Direction::Up });
                if (y > 0)
                    tile.Neighbours.push_back({ m_GridEntities[(y - 1) * m_Config.Width + x], Direction::Down });
                if (x > 0)
                    tile.Neighbours.push_back({ m_GridEntities[y * m_Config.Width + x - 1], Direction::Left });
                if (x < m_Config.Width - 1)
                    tile.Neighbours.push_back({ m_GridEntities[y * m_Config.Width + x + 1], Direction::Right });
            }
        }
    }

    void Grid::GenerateTile(Entity e, char tileChar)
    {
        auto& [src, tile] = e.GetComponents<SpriteRendererComponent, TileComponent>();

        switch (tileChar)
        {
            case '#': // Wall
            {
                // Add rigid body and collider
                auto& rb = e.AddComponent<Rigidbody2DComponent>();
                rb.Gravity = false;
                rb.Type = Rigidbody2DComponent::BodyType::Static;
                auto& box2d = e.AddComponent<BoxCollider2DComponent>();
                box2d.Friction = 0.0f;

                tile.Type = TileType::Wall;
                src.Color = { 0.1f, 0.0f, 1.0f, 1.0f };

                // Store walls only for faster access later
                m_WallEntities.push_back(e);
                break;
            }
            case 'E': // Enemy
            {
                tile.Type = TileType::Empty;
                src.Color = { 0.0f, 0.0f, 0.0f, 1.0f };

                m_DispatchEventCallback(GameEvent::EnemyStartEntityRetrieved, e);

                break;
            }
            case 'P': // Player start position
            {
                tile.Type = TileType::Empty;
                src.Color = { 0.0f, 0.0f, 0.0f, 1.0f };

                // Send position to gameplay manager
                m_DispatchEventCallback(GameEvent::PlayerStartEntityRetrieved, e);

                break;
            }
            case '-': // Empty
            {
                tile.Type = TileType::Empty;
                src.Color = { 0.0f, 0.0f, 0.0f, 1.0f };

                Entity lootEntity = m_Config.CurrentScene->CreateEntity("Point");

                glm::vec2 offset = { m_Config.Width / 2.0f - 0.5f, m_Config.Height / 2.0f - 0.5f };

                auto& transform = lootEntity.Transform();
                transform.Translation = e.Transform().Translation;

                // Draw order
                transform.Translation.z = 0.001f;

                transform.Scale *= 0.25f;

                lootEntity.AddComponent<SpriteRendererComponent>();

                auto& loot = lootEntity.AddCustomComponent<LootComponent>();
                loot.Value = 1;

                // Assign loot to the tile
                tile.LootEntity = lootEntity;
                break;
            }
        }
    }
}
