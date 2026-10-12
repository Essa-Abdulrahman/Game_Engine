#pragma once

#include "Odyssey/Core/Base.h"

#include <string>

namespace Odyssey {

	// Events are dispatched synchronously: when an event is emitted it is handled immediately.
	// Do not keep references to an event beyond the handler call.

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, WindowFileDrop,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory : uint32_t
	{
		EventCategoryNone        = 0,
		EventCategoryApplication = OD_BIT(0),
		EventCategoryInput       = OD_BIT(1),
		EventCategoryKeyboard    = OD_BIT(2),
		EventCategoryMouse       = OD_BIT(3),
		EventCategoryMouseButton = OD_BIT(4)
	};

#define OD_EVENT_CLASS_TYPE(type) \
	static ::Odyssey::EventType GetStaticType() { return ::Odyssey::EventType::type; } \
	::Odyssey::EventType GetEventType() const override { return GetStaticType(); } \
	const char* GetName() const override { return #type; }

#define OD_EVENT_CLASS_CATEGORY(category) \
	uint32_t GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		virtual ~Event() = default;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual uint32_t GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category) const { return (GetCategoryFlags() & category) != 0; }
	public:
		bool Handled = false;
	};

	class EventDispatcher
	{
	public:
		explicit EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		// Calls func if the event is of type T. func returns true if it handled the event.
		// Returns true if func was called. Once an event is marked handled it stays handled.
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() != T::GetStaticType())
				return false;

			const bool handled = func(static_cast<T&>(m_Event));
			m_Event.Handled = m_Event.Handled || handled;
			return true;
		}
	private:
		Event& m_Event;
	};

}
