#pragma once

#include <any>
#include <functional>

namespace PacMan
{
    enum class GameEvent : uint32_t
    {
        PlayerStartEntityRetrieved = 0, // Entity position
        EnemyStartEntityRetrieved,      // vec3 position
        PlayerChangedGridPosition,      // Grid entity under player

        MenuNewGame,
        MenuExit,

        OnCollectibleAcquired,

        GameOverPlayerWon,
        GameOverPlayerLost
    };

    using GameEventCallback = std::function<void(GameEvent, const std::any& data)>;
}
