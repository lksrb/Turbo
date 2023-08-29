#include "tbopch.h"
#include "UserInterfaceLayer.h"

#include "Turbo/Platform/Vulkan/VulkanUserInterfaceLayer.h"

namespace Turbo {

    UserInterfaceLayer* UserInterfaceLayer::Create()
    {
        return new VulkanUserInterfaceLayer();
    }

    void UserInterfaceLayer::SetBlockEvents(bool blockEvents)
    {
        m_BlockEvents = blockEvents;
    }
}

