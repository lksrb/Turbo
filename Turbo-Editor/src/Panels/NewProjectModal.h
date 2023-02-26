#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/Filepath.h"

#include <functional>

namespace Turbo::Ed
{
    struct ProjectInfo
    {
        Filepath RootDirectory;
        FString64 Name;
    };

    using NewProjectCallback = std::function<bool(const ProjectInfo&)>;

    // Modal window
    class NewProjectModal
    {
    public:
        NewProjectModal();
        ~NewProjectModal();

        void SetCallback(const NewProjectCallback& callback);

        void Open();
        void OnUIRender();
    private:
        NewProjectCallback m_Callback;
        bool m_Open;
    };
}
