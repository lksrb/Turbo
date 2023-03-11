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

        bool Deserialize(const std::filesystem::path& filepath);
        bool Serialize(const std::filesystem::path& filepath);
    private:
        Ref<Project> m_Project;
    };

}
