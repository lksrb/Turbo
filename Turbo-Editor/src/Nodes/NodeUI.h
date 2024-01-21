#pragma once

namespace Turbo {
    struct ScriptNode;
    struct ScriptPinLink;
    class UUID;
}

namespace Turbo::UI {
    void DrawNode(const ScriptNode& node);
    void DrawLink(UUID linkUUID, const ScriptPinLink& link);
}
