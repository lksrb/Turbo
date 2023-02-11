#pragma once

#include <iostream>

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Application.h"
#include "Turbo/Core/Memory.h"

#include <crtdbg.h>

extern Turbo::Application* Turbo::CreateApplication();

#ifdef TBO_DEBUG

int main()
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    
//    _CrtSetBreakAlloc(462);

    Turbo::Memory::Initialize();
    {
        Turbo::Engine* engine = Turbo::Engine::Create(Turbo::CreateApplication);
        engine->Run();
        delete engine;
    }
    Turbo::Memory::Shutdown();

    return 0;
}

#elif TBO_RELEASE

int main()
{
    Turbo::Memory::Initialize();
    {
        Turbo::Engine* engine = Turbo::Engine::Create(Turbo::CreateApplication);
        engine->Run();
        delete engine;
    }
    Turbo::Memory::Shutdown();

    return 0;
}

#elif TBO_DIST

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Turbo::Memory::Initialize();
    {
        Turbo::Engine* engine = Turbo::Engine::Create(Turbo::CreateApplication);
        engine->Run();
        delete engine;
    }
    Turbo::Memory::Shutdown();

    return 0;
}

#endif
