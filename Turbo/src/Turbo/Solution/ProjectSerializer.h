#pragma once

#include <Turbo/Core/Common.h>

namespace Turbo 
{
    class Project;

    class ProjectSerializer 
    {
    public:
        ProjectSerializer(Ref<Project> project);
        ~ProjectSerializer();

        bool Deserialize(const std::string& filepath);

        bool Serialize(const std::string& filepath);
    private:
        Ref<Project> m_Project;
    };

}
