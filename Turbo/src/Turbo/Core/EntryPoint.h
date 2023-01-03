#pragma once

#include <iostream>

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Application.h"
#include "Turbo/Core/Memory.h"

#include <crtdbg.h>

extern Turbo::Application* Turbo::CreateApplication();

int main()
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    
    //_CrtSetBreakAlloc(4467);

    Turbo::Memory::Initialize();
    {
        Turbo::Engine* engine = Turbo::Engine::Create(Turbo::CreateApplication);
        engine->Run();
        delete engine;
    }
    Turbo::Memory::Shutdown();

    return 0;
}
