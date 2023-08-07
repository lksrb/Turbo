#include "tbopch.h"
#include "Ref.h"

#include <unordered_set>

namespace Turbo {

    static std::unordered_set<void*> s_LiveReferences;
    static std::mutex s_LiveReferenceMutex;

    void RefUtils::AddToLiveReferences(void* instance)
    {
        std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
        TBO_ENGINE_ASSERT(instance);
        s_LiveReferences.insert(instance);
    }

    void RefUtils::RemoveFromLiveReferences(void* instance)
    {
        std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
        TBO_ENGINE_ASSERT(instance);
        TBO_ENGINE_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end());
        s_LiveReferences.erase(instance);
    }

    bool RefUtils::IsAlive(void* instance)
    {
        TBO_ENGINE_ASSERT(instance);
        return s_LiveReferences.find(instance) != s_LiveReferences.end();
    }

}
