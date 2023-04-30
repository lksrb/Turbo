#pragma once

#include <Turbo/Editor/EditorPanel.h>
#include <Turbo/Renderer/Texture2D.h>
#include <Turbo/Scene/Scene.h>

namespace Turbo::Ed
{
    class ContentBrowserPanel : public EditorPanel
    {
    public:
        ContentBrowserPanel();
        ~ContentBrowserPanel();

        void OnDrawUI() override;
        void OnProjectChanged(const Ref<Project>& project) override;

        void OnSceneContextChanged(const Ref<Scene>& context) override;
    private:
        Ref<Texture2D> m_DirectoryIcon, m_FileIcon;

        Ref<Scene> m_Context;

        std::filesystem::path m_BasePath;
        std::filesystem::path m_CurrentDirectory;
    };
}
