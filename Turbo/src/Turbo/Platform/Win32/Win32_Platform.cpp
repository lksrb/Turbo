#include "tbopch.h"
#include "Turbo/Core/PLATFORM.h"
#include "Turbo/Core/Engine.h"

#include "Win32_Window.h"

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commdlg.h>
//#include <stdio.h>
//#include <tchar.h>
#include <shellapi.h>
#include <cwchar>

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
    }
    struct Win32_Platform
    {
        struct
        {
            u64 Frequency;
            u64 Offset;
        } Timer;
    };

    static Win32_Platform* s_PlatformData = nullptr;

    void Platform::Init()
    {
        s_PlatformData = new Win32_Platform;

        ::QueryPerformanceFrequency((LARGE_INTEGER*)&s_PlatformData->Timer.Frequency);
        ::QueryPerformanceCounter((LARGE_INTEGER*)&s_PlatformData->Timer.Offset);
    }

    void Platform::Shutdown()
    {
        delete s_PlatformData;
    }

    f32 Platform::GetTime()
    {
        uint64_t timerValue;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&timerValue);

        return (static_cast<f32>(timerValue - s_PlatformData->Timer.Offset) / (s_PlatformData->Timer.Frequency));
    }

    std::filesystem::path Platform::OpenFileDialog(const char* title, const char* filter)
    {
        // Title conversion
        size_t title_size = strlen(title) + 1;
        WCHAR wtitle[MAX_PATH] = {};
        mbstowcs_s(NULL, &wtitle[0], title_size, title, title_size - 1);

        // Filter conversion
        size_t filter_size = strlen(filter) + 1;
        WCHAR wfilter[MAX_PATH] = {};
        mbstowcs_s(NULL, &wfilter[0], filter_size, filter, filter_size);

        OPENFILENAME ofn = {};
        WCHAR szFile[MAX_PATH] = { 0 };
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = dynamic_cast<Win32_Window*>(Engine::Get().GetViewportWindow())->GetHandle();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = wfilter;
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = wtitle;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;
        if (::GetOpenFileName(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        TBO_ENGINE_WARN("[Win32_Platform::OpenFileDialog] Cancelled!");
        return {};
    }

    std::filesystem::path Platform::OpenBrowseFolderDialog(const char* title, const char* path)
    {
        // Title conversion
        size_t title_size = strlen(title) + 1;
        WCHAR wtitle[MAX_PATH] = {};
        mbstowcs_s(NULL, &wtitle[0], title_size, title, title_size - 1);

        WCHAR selected_path[MAX_PATH];

        BROWSEINFO bi = { 0 };
        bi.lpszTitle = wtitle;
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_STATUSTEXT;
        bi.lpfn = BrowseCallbackProc;
        bi.lParam = reinterpret_cast<LPARAM>(path);

        LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);

        if (pidl != 0)
        {
            // Get the name of the folder and put it in path
            ::SHGetPathFromIDList(pidl, selected_path);

            // Free memory used
            IMalloc* imalloc = 0;
            if (SUCCEEDED(SHGetMalloc(&imalloc)))
            {
                imalloc->Free(pidl);
                imalloc->Release();
            }

            return selected_path;
        }

        TBO_ENGINE_WARN("[Win32_Platform::OpenBrowseFolderDialog] Cancelled!");

        return {};
    }

    std::filesystem::path Platform::SaveFileDialog(const char* filter, const char* suffix)
    {
        // Filter conversion
        size_t filter_size = strlen(filter) + 1;
        WCHAR wfilter[MAX_PATH] = {};
        mbstowcs_s(NULL, &wfilter[0], filter_size, filter, filter_size);

        // Suffix conversion
        size_t suffix_size = strlen(suffix) + 1;
        WCHAR wsuffix[MAX_PATH] = {};
        mbstowcs_s(NULL, &wsuffix[0], suffix_size, suffix, suffix_size - 1);

        std::filesystem::path selected_path;

        // Opening save dialog
        OPENFILENAME ofn;
        WCHAR szFile[MAX_PATH] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = dynamic_cast<Win32_Window*>(Engine::Get().GetViewportWindow())->GetHandle();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = wfilter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (::GetSaveFileName(&ofn) == TRUE)
        {
            selected_path = ofn.lpstrFile;
            selected_path.concat(suffix);
            return selected_path;
        }

        TBO_ENGINE_WARN("[Win32_Platform::SaveFileDialog] Cancelled!");
        return selected_path;
    }

    bool Platform::OpenFileExplorer(const std::filesystem::path& directory)
    {
        // Construct argument list
        WCHAR args[MAX_PATH] = {};
        //wcscat_s(args, L"/select,");
        wcscat_s(args, directory.wstring().c_str());

        // Execute Windows Explorer
        HINSTANCE result = ShellExecute(NULL, L"open", L"explorer", args, NULL, SW_SHOWNORMAL);
        if ((intptr_t)(result) <= HINSTANCE_ERROR)
        {
            DWORD errorCode = GetLastError();
            TBO_ENGINE_ERROR("Could not open the file explorer! ErrorCode: {}", errorCode);

            return false;
        }

        return true;
    }

    // TODO: Rework this
    bool Platform::Execute(const std::string& appName, const std::string& args, const std::string& currentPath, bool wait)
    {
        STARTUPINFOA si = {};
        si.cb = sizeof(si);

        LPCSTR currentDirectory = NULL;
        if(!currentPath.empty())
            currentDirectory = currentPath.c_str();

        PROCESS_INFORMATION pi = {};

        CHAR szCmd[MAX_PATH] = { 0 };
        //strcat_s(szCmd, "cmd.exe /C start ");
        strcat_s(szCmd, appName.c_str());
        strcat_s(szCmd, " ");
        strcat_s(szCmd, args.c_str());
        // Start the child process. 
        if (!CreateProcessA(NULL,   // No module name (use command line)
            szCmd,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            currentDirectory,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            TBO_ENGINE_ERROR("CreateProcess failed! ErrorCode: {0}", GetLastError());
            return false;
        }

        // Wait until child process exits.
        if(wait)
            WaitForSingleObject(pi.hProcess, INFINITE); // Infinite is too much

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;
    }
}
