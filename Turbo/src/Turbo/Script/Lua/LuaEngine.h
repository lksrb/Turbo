#pragma once

#include "Turbo/Core/Common.h"

#include "Turbo/Core/Filepath.h"

struct lua_State;

namespace Turbo
{
	enum class StackLevel : u32
	{
		None = 0,
		ProjectLevel,
		SceneLevel,
        EntityLevel,
        ComponentLevel,
        ParameterLevel
	};

    using CommandCallback = void(*)(void*, const char**, size_t size);

    class LuaEngine
    {
    public:
        LuaEngine();
        ~LuaEngine();

        void SetUserdata(void* userdata);
        void AddCommand(const FString32& keyword, CommandCallback callback);
        bool Execute(const Filepath& filePath);
    private:
        void Initialize();
    private:
        lua_State* m_LuaState;
    };

}
