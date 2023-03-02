#include "tbopch.h"
#include "LuaEngine.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define L m_LuaState

namespace Turbo
{
    static void* l_alloc(void* ud, void* ptr, size_t osize, size_t nsize)
    {
        (void)ud; (void)osize;  /* not used */
        if (nsize == 0)
        {
            free(ptr);
            return NULL;
        }
        else
            return realloc(ptr, nsize);
    }

    LuaEngine::LuaEngine()
        : m_LuaState(nullptr)
    {
        Initialize();
    }

    LuaEngine::~LuaEngine()
    {
        lua_close(m_LuaState);
    }

    void LuaEngine::AddCommand(const String32& keyword, CommandCallback callback)
    {
        // Register new function and push upvalues to it
        lua_pushlightuserdata(L, callback); // <-- Up value
        lua_pushcclosure(L, [](lua_State* L)
        {
            if (lua_gettop(L) != 1)
            {
                lua_Debug debug;
                lua_getstack(L, 0, &debug);
                TBO_ENGINE_ERROR("Too few arguments! Line: {0}", debug.currentline);
                //luaL_argerror
                return -1;
            }
            
            // Retrieve upvalue a.k.a. the callback
            CommandCallback callback = (CommandCallback)lua_touserdata(L, lua_upvalueindex(1));

            lua_getglobal(L, "Turbo");
            int index = lua_gettop(L);
            lua_getfield(L, index, "UserData");
            void* userData = lua_touserdata(L, -1);
            lua_pop(L, 2);

            int type = lua_type(L, 1);
            
            // Number of string arguments, to differentiate between single string and multistring command
            int strArgs = lua_gettop(L);

            // Single string
            if (type == LUA_TSTRING)
            {
                const char* str = lua_tostring(L, -1);
               
                callback(userData, &str, 1);
            }
            else // Table
            {
                std::vector<const char*> args;

                // Read whole table

                lua_pushnil(L); 
                while (lua_next(L, 1) != 0)
                {
                    const char* value = lua_tostring(L, -1);
                    args.push_back(value);

                    lua_pop(L, 1);
                }
                callback(userData, args.data(), args.size());
            }


        return 0;
        }, 1);
        lua_setglobal(L, keyword.CStr());
    }

    void LuaEngine::Initialize()
    {
        m_LuaState = lua_newstate(l_alloc, nullptr);
        luaL_openlibs(L);

        lua_newtable(L);
        lua_setglobal(L, "Turbo");
    }

    bool LuaEngine::Execute(const Filepath& filePath)
    {
        // Execute the script
        if (luaL_dofile(L, filePath.CStr()) != LUA_OK)
        {
            const char* errormsg = lua_tostring(L, -1);
            
            TBO_ENGINE_ERROR("[Lua] {0}", errormsg);

            return false;
        }

        return true;
    }

    void LuaEngine::SetUserdata(void* userdata)
    {
        lua_getglobal(L, "Turbo");

        lua_pushstring(L, "UserData");
        lua_pushlightuserdata(L, userdata);
        lua_settable(L, -3);

        lua_pop(L, 1);
    }

}

