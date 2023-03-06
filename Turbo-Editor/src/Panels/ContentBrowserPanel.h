#pragma once

#include "Panel.h"

#include <Turbo/Core/Filepath.h>
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
    private:
        Ref<Texture2D> m_DirectoryIcon, m_FileIcon;

        Filepath m_CurrentDirectory;
    };
}
