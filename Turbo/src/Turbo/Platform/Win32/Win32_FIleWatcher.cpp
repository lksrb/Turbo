#include "tbopch.h"
#include "Turbo/Core/FileWatcher.h"

#include <Windows.h>

namespace Turbo
{
    FileWatcher::FileWatcher(NotifyEvent events, bool search_tree, const Callback& callback)
        : m_NotifyEvents(events), m_SearchTree(search_tree), m_OnDirChangedCallback(callback)
    {
    }

    FileWatcher::~FileWatcher()
    {
        StopWatching();

        if (m_Thread.joinable())
            m_Thread.join();

        // Clean up event handle
        if (m_TerminateThreadHandle)
        {
            ::CloseHandle(m_TerminateThreadHandle);
            m_TerminateThreadHandle = nullptr;
        }

        // Clean up file handle
        if (m_FileHandle)
        {
            ::CloseHandle(m_FileHandle);
            m_FileHandle = nullptr;
        }
    }

    void FileWatcher::Watch(const std::filesystem::path& watchPath)
    {
        TBO_ENGINE_ASSERT(!m_IsWatching, "FileWatcher is already watching!");
        TBO_ENGINE_ASSERT(m_OnDirChangedCallback, "Callback is not set!");

        // Set watch path
        m_WatchPath = watchPath;

        // Get file handle
        m_FileHandle = ::CreateFile(m_WatchPath.wstring().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
        if (m_FileHandle == INVALID_HANDLE_VALUE)
        {
            m_FileHandle = nullptr;
            TBO_ENGINE_ERROR("Could not open the file! ({0})", m_WatchPath);
            return;
        }

        // Create TerminateThread Event
        m_TerminateThreadHandle = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (m_TerminateThreadHandle == INVALID_HANDLE_VALUE)
        {
            m_TerminateThreadHandle = nullptr;
            TBO_ENGINE_ERROR("Could not create event handle!");
            return;
        }

        m_IsWatching = true;

        // Start separate thread
        m_Thread = std::thread([&]()
        {
            // Create OnDirectoryChange Event
            OVERLAPPED OnDirChange = {};
            OnDirChange.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

            if (OnDirChange.hEvent == nullptr)
            {
                TBO_ENGINE_ERROR("Could not create event handle!");
                ::CloseHandle(m_FileHandle);
                return;
            }
            
            BYTE buffer[4096];

            while (true)
            {
                DWORD bytesRead;
                bool success = ::ReadDirectoryChangesW(m_FileHandle, buffer, sizeof(buffer), m_SearchTree, static_cast<DWORD>(m_NotifyEvents), &bytesRead, &OnDirChange, NULL);

                if (!success)
                {
                    TBO_ENGINE_ERROR("Error: ReadDirectoryChangesW failed"); // TODO: More descriptive error handling
                    break;
                }

                //TBO_ENGINE_TRACE("Waiting for events...");
                HANDLE handles[] = { OnDirChange.hEvent, m_TerminateThreadHandle };
                DWORD result = ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);

                if (result == WAIT_OBJECT_0)
                {
                    success = ::GetOverlappedResult(
                        m_FileHandle,
                        &OnDirChange,
                        &bytesRead,
                        TRUE);

                    // Retrieve the details of the file change
                    FILE_NOTIFY_INFORMATION* fileInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

                    // If is the buffer is not zero initialized, do this
                    if (fileInfo->Action)
                    {
                        const auto& path_to_event_file = m_WatchPath / std::wstring(fileInfo->FileName, fileInfo->FileNameLength / sizeof(WCHAR));
                        m_OnDirChangedCallback(path_to_event_file, static_cast<FileWatcher::FileEvent>(fileInfo->Action)); // TODO: Should be a convert function
                    }
                }
                else // (WAIT_OBJECT_0 + 1) Second handle was signaled
                {
                    break;
                }
            }

            // Clean up OnDirChanged Event
            if (OnDirChange.hEvent)
            {
                ::CloseHandle(OnDirChange.hEvent);
                OnDirChange.hEvent = nullptr;
            }
        });
    }

    void FileWatcher::StopWatching()
    {
        if (m_IsWatching)
        {
            m_IsWatching = false;

            // Signal event to close thread
            ::SetEvent(m_TerminateThreadHandle);
        }
    }


}
