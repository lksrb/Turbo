#include "tbopch.h"
#include "LayerStack.h"

namespace Turbo {

    void LayerStack::PushLayer(Layer* layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
    }

    void LayerStack::PushOverlay(Layer* layer)
    {
        m_Layers.push_back(layer);
    }

    void LayerStack::PopLayer(Layer* layer)
    {

    }

}
