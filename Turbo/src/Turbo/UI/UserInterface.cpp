#include "tbopch.h"
#include "UserInterface.h"

#include "Turbo/Platform/Vulkan/VulkanUserInterface.h"

namespace Turbo
{
	void UserInterface::SetBlockEvents(bool blockEvents)
	{
        m_BlockEvents = blockEvents;
	}

	Scope<UserInterface> UserInterface::Create()
    {
        return CreateScope<VulkanUserInterface>();
    }
}

