#include "tbopch.h"
#include "Material.h"

#include "Turbo/Platform/Vulkan/VulkanMaterial.h"

namespace Turbo
{
    Material::Material(const Material::Config& config)
        : m_Config(config)
    {
    }

    Material::~Material()
    {
    }

    Material* Material::Create(const Material::Config& config)
    {
        return new VulkanMaterial(config);
    }

}

