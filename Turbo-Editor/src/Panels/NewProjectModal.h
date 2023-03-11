#pragma once

#include <filesystem>
#include <functional>

namespace Turbo::Ed
{
    struct ProjectInfo
    {
        std::filesystem::path RootDirectory;
        std::string Name;
    };

    using NewProjectCallback = std::function<void(const ProjectInfo&)>;

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
