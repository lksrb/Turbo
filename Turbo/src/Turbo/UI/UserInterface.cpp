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

	void UserInterface::SetBlockEvents(bool block_events)
	{
        m_BlockEvents = block_events;
	}

	Ref<UserInterface> UserInterface::Create()
    {
        return Ref<VulkanUserInterface>::Create();
    }
}

