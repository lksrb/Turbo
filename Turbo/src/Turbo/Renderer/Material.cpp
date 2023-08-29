#include "tbopch.h"
#include "Material.h"

#include "Turbo/Platform/Vulkan/VulkanMaterial.h"

namespace Turbo
{
    Material::Material(const Ref<Shader>& shader)
        : m_MaterialShader(shader)
    {
    }

    Material::~Material()
    {
    }

    Ref<Material> Material::Create(const Ref<Shader>& shader)
    {
        return Ref<VulkanMaterial>::Create(shader);
    }

}

