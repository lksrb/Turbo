#pragma once

#include "Turbo/Core/Engine.h"
#include "Turbo/Core/Application.h"
#include "Turbo/Core/Memory.h"

extern Turbo::Application* Turbo::CreateApplication();

#ifdef TBO_DEBUG

int main(int argc, char* argv[])
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

#elif TBO_RELEASE

int main(int argc, char* argv[])
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
