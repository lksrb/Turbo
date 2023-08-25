#include "tbopch.h"
#include "Project.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/SceneSerializer.h"
#include "Turbo/Solution/ProjectSerializer.h"

namespace Turbo
{
    Project::Project()
    {
    }

    Project::~Project()
    {
    }

    void Project::SetActive(Ref<Project> project)
    {
        s_ActiveProject = project;

        if (s_ActiveProject)
        {
            s_ActiveProject->m_AssetRegistry = Ref<EditorAssetRegistry>::Create();
        }
    }

}
