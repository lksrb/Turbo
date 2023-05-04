#pragma once

#include "Common.h"

#include <filesystem>
#include <functional>
#include <thread>

namespace Turbo
{
    class FileWatcher
    {
    public:
        // From windows.h
        enum FileEvent_ : u32
        {
            FileEvent_None = 0x0000000,
            FileEvent_Added = 0x00000001,
            FileEvent_Removed = 0x00000002,
            FileEvent_Modified = 0x00000003,
            FileEvent_RenamedOld = 0x00000004,
            FileEvent_RenamedNew = 0x00000005
        };

        // From windows.h
        enum NotifyEvent_ : u32
        {
            NotifyEvent_None = 0,
            NotifyEvent_FileName = 0x00000001,
            NotifyEvent_DirName = 0x00000002,
            NotifyEvent_Attributes = 0x00000004,
            NotifyEvent_Size = 0x00000008,
            NotifyEvent_LastWrite = 0x000000010,
            NotifyEvent_LastAccess = 0x00000020,
            NotifyEvent_Creation = 0x000000040,
            NotifyEvent_Security = 0x00000100,

            NotifyEvent_All = NotifyEvent_FileName | NotifyEvent_DirName | NotifyEvent_Attributes | NotifyEvent_Size | NotifyEvent_LastWrite | NotifyEvent_LastAccess | NotifyEvent_Creation | NotifyEvent_Security
        };

        using NotifyEvent = u32;
        using FileEvent = u32;
        using Callback = std::function<void(std::filesystem::path, FileWatcher::FileEvent)>;

        FileWatcher(NotifyEvent events, bool searchTree, const Callback& callback);
        ~FileWatcher();

        void Watch(const std::filesystem::path& watchPath);

        void StopWatching();
    private:
        bool m_SearchTree = false;
        bool m_IsWatching = false;
        NotifyEvent m_NotifyEvents = NotifyEvent_None;
        void* m_TerminateThreadHandle = nullptr;
        void* m_FileHandle = nullptr;
        std::thread m_Thread;
        Callback m_OnDirChangedCallback;
        std::filesystem::path m_WatchPath;
    };

}
