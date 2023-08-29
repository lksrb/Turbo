#include "tbopch.h"
#include "MaterialAsset.h"

#include "ShaderLibrary.h"

#include <glm/gtc/type_ptr.hpp>

namespace Turbo {

    MaterialAsset::MaterialAsset()
    {
        m_Material = Material::Create({ ShaderLibrary::Get("StaticMesh") });
        SetAlbedoColor(glm::vec3(1.0f));
    }

    void MaterialAsset::SetAlbedoColor(const glm::vec3& albedoColor)
    {
        m_Material->Set("p_Material.AlbedoColor", glm::value_ptr(albedoColor), sizeof(glm::vec3));
    }

}



