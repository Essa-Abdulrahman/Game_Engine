#pragma once

#include "Odyssey/Core/KeyCodes.h"
#include "Odyssey/Events/Event.h"

#include <format>

namespace Odyssey {

	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }

		OD_EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		explicit KeyEvent(KeyCode keyCode)
			: m_KeyCode(keyCode)
		{
		}
	protected:
		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keyCode, bool isRepeat = false)
			: KeyEvent(keyCode), m_IsRepeat(isRepeat)
		{
		}

		bool IsRepeat() const { return m_IsRepeat; }

		std::string ToString() const override { return std::format("KeyPressedEvent: {} (repeat = {})", KeyCodeToString(m_KeyCode), m_IsRepeat); }

		OD_EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		explicit KeyReleasedEvent(KeyCode keyCode)
			: KeyEvent(keyCode)
		{
		}

		std::string ToString() const override { return std::format("KeyReleasedEvent: {}", KeyCodeToString(m_KeyCode)); }

		OD_EVENT_CLASS_TYPE(KeyReleased)
	};

	// Text input. Carries a Unicode code point rather than a physical key.
	class KeyTypedEvent : public Event
	{
	public:
		explicit KeyTypedEvent(uint32_t codepoint)
			: m_Codepoint(codepoint)
		{
		}

		uint32_t GetCodepoint() const { return m_Codepoint; }

		std::string ToString() const override { return std::format("KeyTypedEvent: U+{:04X}", m_Codepoint); }

		OD_EVENT_CLASS_TYPE(KeyTyped)
		OD_EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	private:
		uint32_t m_Codepoint;
	};

}
