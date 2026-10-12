#pragma once

#include "Odyssey/Core/KeyCodes.h"
#include "Odyssey/Events/Event.h"

#include <format>

namespace Odyssey {

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y)
		{
		}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string ToString() const override { return std::format("MouseMovedEvent: {}, {}", m_MouseX, m_MouseY); }

		OD_EVENT_CLASS_TYPE(MouseMoved)
		OD_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_MouseX;
		float m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset)
		{
		}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override { return std::format("MouseScrolledEvent: {}, {}", m_XOffset, m_YOffset); }

		OD_EVENT_CLASS_TYPE(MouseScrolled)
		OD_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_XOffset;
		float m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

		OD_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
	protected:
		explicit MouseButtonEvent(MouseCode button)
			: m_Button(button)
		{
		}
	protected:
		MouseCode m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		explicit MouseButtonPressedEvent(MouseCode button)
			: MouseButtonEvent(button)
		{
		}

		std::string ToString() const override { return std::format("MouseButtonPressedEvent: {}", MouseCodeToString(m_Button)); }

		OD_EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		explicit MouseButtonReleasedEvent(MouseCode button)
			: MouseButtonEvent(button)
		{
		}

		std::string ToString() const override { return std::format("MouseButtonReleasedEvent: {}", MouseCodeToString(m_Button)); }

		OD_EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}
