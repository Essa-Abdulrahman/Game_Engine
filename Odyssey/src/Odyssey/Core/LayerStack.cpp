#include "odpch.h"
#include "Odyssey/Core/LayerStack.h"

#include "Odyssey/Core/Assert.h"

#include <algorithm>
#include <iterator>

namespace Odyssey {

	LayerStack::~LayerStack()
	{
		Clear();
	}

	Layer* LayerStack::PushLayer(Scope<Layer> layer)
	{
		OD_CORE_VERIFY(layer != nullptr, "Cannot push a null layer");

		Layer* rawLayer = layer.get();
		m_Layers.insert(m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex), std::move(layer));
		m_LayerInsertIndex++;
		rawLayer->OnAttach();
		return rawLayer;
	}

	Layer* LayerStack::PushOverlay(Scope<Layer> overlay)
	{
		OD_CORE_VERIFY(overlay != nullptr, "Cannot push a null overlay");

		Layer* rawOverlay = overlay.get();
		m_Layers.push_back(std::move(overlay));
		rawOverlay->OnAttach();
		return rawOverlay;
	}

	Scope<Layer> LayerStack::PopLayer(Layer* layer)
	{
		Scope<Layer> removed = Remove(layer, 0, m_LayerInsertIndex);
		if (removed)
			m_LayerInsertIndex--;
		return removed;
	}

	Scope<Layer> LayerStack::PopOverlay(Layer* overlay)
	{
		return Remove(overlay, m_LayerInsertIndex, m_Layers.size());
	}

	void LayerStack::Clear()
	{
		while (!m_Layers.empty())
		{
			m_Layers.back()->OnDetach();
			m_Layers.pop_back();
		}
		m_LayerInsertIndex = 0;
	}

	Scope<Layer> LayerStack::Remove(Layer* layer, size_t rangeBegin, size_t rangeEnd)
	{
		if (layer == nullptr)
			return nullptr;

		const auto first = m_Layers.begin() + static_cast<std::ptrdiff_t>(rangeBegin);
		const auto last = m_Layers.begin() + static_cast<std::ptrdiff_t>(rangeEnd);
		const auto it = std::find_if(first, last, [layer](const Scope<Layer>& entry) { return entry.get() == layer; });
		if (it == last)
			return nullptr;

		Scope<Layer> removed = std::move(*it);
		m_Layers.erase(it);
		removed->OnDetach();
		return removed;
	}

}
