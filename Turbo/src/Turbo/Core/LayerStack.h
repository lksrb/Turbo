#pragma once

#include "Layer.h"

namespace Turbo {

    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack() = default;

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);
        
        auto begin() { return m_Layers.begin(); }
        auto end() { return m_Layers.end(); }

        auto rbegin() { return m_Layers.rbegin(); }
        auto rend() { return m_Layers.rend(); }
    private:
        u32 m_LayerInsertIndex = 0;
        std::vector<Layer*> m_Layers;
    };

}
