#include "tbopch.h"
#include "UserInterface.h"

#include "Turbo/Platform/Vulkan/VulkanUserInterface.h"

namespace Turbo
{
    UserInterface::UserInterface()
    {
    }

    UserInterface::~UserInterface()
    {
    }

    UserInterface* UserInterface::Create()
    {
        return new VulkanUserInterface();
    }
}

