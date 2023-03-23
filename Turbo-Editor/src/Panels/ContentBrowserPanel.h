#pragma once

#include <Turbo/Editor/EditorPanel.h>
#include <Turbo/Renderer/Texture2D.h>

namespace Turbo::Ed
{
    class ContentBrowserPanel : public EditorPanel
    {
    public:
        ContentBrowserPanel();
        ~ContentBrowserPanel();

        void OnDrawUI() override;
        void OnProjectChanged(const Ref<Project>& project) override;
    private:
        Ref<Texture2D> m_DirectoryIcon, m_FileIcon;

        std::filesystem::path m_BasePath;
        std::filesystem::path m_CurrentDirectory;
    };
}
