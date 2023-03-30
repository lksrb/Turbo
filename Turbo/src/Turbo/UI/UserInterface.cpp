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

	void UserInterface::SetBlockEvents(bool blockEvents)
	{
        m_BlockEvents = blockEvents;
	}

	Ref<UserInterface> UserInterface::Create()
    {
        return Ref<VulkanUserInterface>::Create();
    }
}

