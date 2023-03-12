#pragma once

#include "Panel.h"

#include <Turbo//Renderer/Texture2D.h>

namespace Turbo::Ed
{
    class ContentBrowserPanel : public Panel
    {
    public:
        ContentBrowserPanel();
        ~ContentBrowserPanel();

        void OnDrawUI() override;
        void OnEvent(Event& e) override;
        void SetProjectAssetPath();
    private:
        Ref<Texture2D> m_DirectoryIcon, m_FileIcon;

        std::filesystem::path m_CurrentDirectory;
    };
}
