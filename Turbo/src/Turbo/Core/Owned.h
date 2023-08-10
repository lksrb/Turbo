#pragma once

#include <memory>

namespace Turbo {

    // TODO: Owned class

    template<typename T>
    using Owned = std::unique_ptr<T>;

    template<typename T, typename ... Args>
    constexpr inline Owned<T> CreateOwned(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}
