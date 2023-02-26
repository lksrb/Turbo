#include "tbopch.h"
#include "Turbo/Core/PLATFORM.h"
#include "Turbo/Core/Engine.h"

#include "Win32_Window.h"

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commdlg.h>

#define CheckPLATFORMError CheckError()

namespace Turbo
{
    namespace
    {
        static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
        {
            if (uMsg == BFFM_INITIALIZED)
            {
                LPCSTR path = reinterpret_cast<LPCSTR>(lpData);
                ::SendMessage(hwnd, BFFM_SETSELECTIONA, TRUE, (LPARAM)path);
            }

            return 0;
        }

        Platform::Result CheckError()
        {
            DWORD error = ::GetLastError();
            switch (error)
            {
                case S_OK: return Platform::Result::Success;
                case ERROR_ALREADY_EXISTS: return Platform::Result::AlreadyExists;
                case ERROR_PATH_NOT_FOUND: return Platform::Result::PathNotFound;
            }

            return Platform::Result::Error;
        }
    }
    struct Win32_Platform
    {
        struct
        {
            u64 Frequency;
            u64 Offset;
        } Timer;
    };

    static Win32_Platform* I;

    void Platform::Initialize()
    {
        I = new Win32_Platform;

        ::QueryPerformanceFrequency((LARGE_INTEGER*)&I->Timer.Frequency);
        ::QueryPerformanceCounter((LARGE_INTEGER*)&I->Timer.Offset);
    }

    void Platform::Shutdown()
    {
        delete I;
    }

    f32 Platform::GetTime()
    {
        uint64_t timerValue;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&timerValue);

        return (f32)(timerValue - I->Timer.Offset) / (I->Timer.Frequency);
    }

    Platform::Result Platform::CreateFile(const Filepath& rootPath, const char* filename, const char* extension)
    {
        Filepath filepath = rootPath;
        filepath /= filename;

        filepath.Append(extension);

        ::CreateFileA(filepath.CStr(),
            GENERIC_READ | GENERIC_WRITE,
            NULL,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (::GetLastError() == ERROR_FILE_EXISTS)
        {
            ::CreateFileA(filepath.CStr(),
            GENERIC_READ | GENERIC_WRITE,
            NULL,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        }

        return CheckPLATFORMError;
    }

    Platform::Result Platform::CreateDirectory(const Filepath& path)
    {
        ::CreateDirectoryA(path.CStr(), NULL);
        return CheckPLATFORMError;
    }

    bool Platform::IsDirectoryEmpty(const Filepath& path)
    {
        return ::PathIsDirectoryEmptyA(path.CStr());
    }

    bool Platform::PathExists(const Filepath& path)
    {
        return ::PathFileExistsA(path.CStr());
    }


    Filepath Platform::GetCurrentPath()
    {
        CHAR szFile[MAX_PATH] = { 0 };
        ::GetCurrentDirectoryA(sizeof(szFile), szFile);

        return szFile;
    }

    Filepath Platform::OpenFileDialog(const char* title, const char* filter)
    {
        OPENFILENAMEA ofn = {};
        CHAR szFile[MAX_PATH] = { 0 };
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = dynamic_cast<Win32_Window*>(Engine::Get().GetViewportWindow())->GetHandle();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = title;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;
        if (::GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        TBO_ENGINE_WARN("Could not open the file!");
        return {};
    }

    Filepath Platform::OpenBrowseFolderDialog(const char* title, const char* savedPath)
    {
        CHAR path[MAX_PATH];

        BROWSEINFOA bi = { 0 };
        bi.lpszTitle = title;
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_STATUSTEXT;
        bi.lpfn = BrowseCallbackProc;
        bi.lParam = reinterpret_cast<LPARAM>(savedPath);

        LPITEMIDLIST pidl = ::SHBrowseForFolderA(&bi);

        if (pidl != 0)
        {
            //get the name of the folder and put it in path
            ::SHGetPathFromIDListA(pidl, path);

            //free memory used
            IMalloc* imalloc = 0;
            if (SUCCEEDED(SHGetMalloc(&imalloc)))
            {
                imalloc->Free(pidl);
                imalloc->Release();
            }

            return path;
        }

        return {};
    }

    void Platform::SetCurrentPath(const Filepath& path)
    {
        TBO_ENGINE_ASSERT(!path.Empty());
        ::SetCurrentDirectoryA(path.CStr());
    }

    Filepath Platform::SaveFileDialog(const char* filter, const char* suffix)
    {
        std::string filepath{};

        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = dynamic_cast<Win32_Window*>(Engine::Get().GetViewportWindow())->GetHandle();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (::GetSaveFileNameA(&ofn) == TRUE)
        {
            filepath = ofn.lpstrFile;
            filepath += suffix;
            return filepath;
        }
        TBO_ENGINE_WARN("Could not save the file!");
        return filepath;
    }
}
