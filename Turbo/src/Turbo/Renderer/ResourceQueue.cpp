#include "tbopch.h"

#include "ResourceQueue.h"
#include "RendererContext.h"

namespace Turbo
{
    ResourceQueue::ResourceQueue()
    {
    }

    ResourceQueue::~ResourceQueue()
    {
    }

    bool ResourceQueue::Execute(ExecutionOrder order)
    {
        if (m_Queue.empty())
        {
            return false;
        }
        std::sort(m_Queue.begin(), m_Queue.end());

        if (order == ExecutionOrder::Allocate)
        {
            for (auto& it = m_Queue.rbegin(); it != m_Queue.rend(); ++it)
            {
                it->Function();
            }
        }
        else // order == ExecutionOrder::Free
        {
            for (auto& it = m_Queue.begin(); it != m_Queue.end(); ++it)
            {
                it->Function();
            }
        }

        m_Queue.clear();

        return true;
    }
}
