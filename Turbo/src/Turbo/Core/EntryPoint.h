#pragma once

#include "Turbo/Core/Application.h"
#include "Turbo/Core/Memory.h"

extern Turbo::Application* Turbo::CreateApplication();

int main(int argc, char* argv[])
{
#if TBO_TRACK_MEMORY
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    {
        auto app = Turbo::CreateApplication();
        app->Run();
        delete app;
    }

    return 0;
}
