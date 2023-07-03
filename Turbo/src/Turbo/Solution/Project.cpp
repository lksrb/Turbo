#include "tbopch.h"
#include "Project.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/SceneSerializer.h"

#include "Turbo/Asset/EditorAssetManager.h"

#include "Turbo/Solution/ProjectSerializer.h"

namespace Turbo
{
    Project::Project(const Project::Config& config)
        : m_Config(config)
    {
        m_AssetManager = Ref<EditorAssetManager>::Create();
    }

    Project::~Project()
    {
    }

	void Project::Build()
	{

	}

}
