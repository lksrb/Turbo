#include "tbopch.h"
#include "UserInterface.h"

#include "Turbo/Platform/Vulkan/VulkanUserInterface.h"

namespace Turbo
{
	void UserInterface::SetBlockEvents(bool blockEvents)
	{
        m_BlockEvents = blockEvents;
	}

	Owned<UserInterface> UserInterface::Create()
    {
        return CreateOwned<VulkanUserInterface>();
    }
}

