#pragma once

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Application.h"
#include "Turbo/Core/Memory.h"

extern Turbo::Application* Turbo::CreateApplication();

#ifdef TBO_DEBUG

int main(int argc, char* argv[])
{
#if TBO_TRACK_MEMORY
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    {
        Turbo::Engine* engine = new Turbo::Engine(Turbo::CreateApplication);
        engine->Run();
        delete engine;
    }

    return 0;
}

#elif TBO_RELEASE

int main(int argc, char* argv[])
{
    Turbo::Engine* engine = Turbo::Engine::Create(Turbo::CreateApplication);
    engine->Run();
    delete engine;

    return 0;
}

#elif TBO_DIST

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Turbo::Engine* engine = Turbo::Engine::Create(Turbo::CreateApplication);
    engine->Run();
    delete engine;

    return 0;
}

#endif
