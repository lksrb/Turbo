#pragma once

#include <Turbo/Core/Filepath.h>

namespace Turbo 
{
    class Project;

    class ProjectSerializer 
    {
    public:
        ProjectSerializer(Ref<Project> project);
        ~ProjectSerializer();

        bool Deserialize(const Filepath& filepath);

        bool Serialize(const Filepath& filepath);
    private:
        Ref<Project> m_Project;
    };

}
