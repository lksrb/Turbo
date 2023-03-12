#include "tbopch.h"
#include "Project.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/SceneSerializer.h"

#include "Turbo/Solution/ProjectSerializer.h"
#include "Turbo/Benchmark/ScopeTimer.h"

namespace Turbo
{
    Project::Project(const Project::Config& config)
        : m_Config(config)
    {
    }

    Project::~Project()
    {
    }
}
