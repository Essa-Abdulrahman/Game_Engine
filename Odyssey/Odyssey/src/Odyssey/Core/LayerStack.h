#pragma once

#include "Odyssey/Core/Base.h"
#include "Odyssey/Core/Layer.h"

#include <vector>

namespace Odyssey {

	// Ordered collection of layers. Regular layers come first (in push order), followed by overlays (in push order).
	// Update iteration runs front to back; event propagation should run back to front so overlays see events first.
	// The stack must not be modified while it is being iterated.
	class LayerStack
	{
	public:
		using LayerList = std::vector<Scope<Layer>>;

		LayerStack() = default;
		// Detaches and destroys all layers, top-most first.
		~LayerStack();

		LayerStack(const LayerStack&) = delete;
		LayerStack& operator=(const LayerStack&) = delete;

		// Takes ownership and calls OnAttach(). Returns a non-owning pointer to the layer.
		Layer* PushLayer(Scope<Layer> layer);
		Layer* PushOverlay(Scope<Layer> overlay);

		// Calls OnDetach() and returns ownership to the caller. Returns null if the layer is not in this stack
		// (or, for PopOverlay, is a regular layer and vice versa).
		Scope<Layer> PopLayer(Layer* layer);
		Scope<Layer> PopOverlay(Layer* overlay);

		// Detaches and destroys all layers, top-most first.
		void Clear();

		size_t GetSize() const { return m_Layers.size(); }
		size_t GetLayerCount() const { return m_LayerInsertIndex; }
		size_t GetOverlayCount() const { return m_Layers.size() - m_LayerInsertIndex; }
		bool IsEmpty() const { return m_Layers.empty(); }

		LayerList::iterator begin() { return m_Layers.begin(); }
		LayerList::iterator end() { return m_Layers.end(); }
		LayerList::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		LayerList::reverse_iterator rend() { return m_Layers.rend(); }

		LayerList::const_iterator begin() const { return m_Layers.begin(); }
		LayerList::const_iterator end() const { return m_Layers.end(); }
		LayerList::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		LayerList::const_reverse_iterator rend() const { return m_Layers.rend(); }
	private:
		Scope<Layer> Remove(Layer* layer, size_t rangeBegin, size_t rangeEnd);
	private:
		LayerList m_Layers;
		size_t m_LayerInsertIndex = 0;
	};

}
