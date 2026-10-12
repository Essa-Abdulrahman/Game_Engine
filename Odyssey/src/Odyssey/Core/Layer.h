#pragma once

#include "Odyssey/Core/Base.h"
#include "Odyssey/Core/Timestep.h"
#include "Odyssey/Events/Event.h"

#include <string>

namespace Odyssey {

	// A slice of application behaviour (eg. the editor, a game, a debug overlay) that receives
	// lifecycle callbacks, per-frame updates and events. Owned by a LayerStack.
	class Layer
	{
	public:
		explicit Layer(std::string name = "Layer");
		virtual ~Layer() = default;

		Layer(const Layer&) = delete;
		Layer& operator=(const Layer&) = delete;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event&) {}

		const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};

}
