#pragma once

#include "Odyssey/Events/Event.h"

#include <filesystem>
#include <format>
#include <utility>
#include <vector>

namespace Odyssey {

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height)
		{
		}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		std::string ToString() const override { return std::format("WindowResizeEvent: {}, {}", m_Width, m_Height); }

		OD_EVENT_CLASS_TYPE(WindowResize)
		OD_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		uint32_t m_Width;
		uint32_t m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		OD_EVENT_CLASS_TYPE(WindowClose)
		OD_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowFocusEvent : public Event
	{
	public:
		WindowFocusEvent() = default;

		OD_EVENT_CLASS_TYPE(WindowFocus)
		OD_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowLostFocusEvent : public Event
	{
	public:
		WindowLostFocusEvent() = default;

		OD_EVENT_CLASS_TYPE(WindowLostFocus)
		OD_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowMovedEvent : public Event
	{
	public:
		WindowMovedEvent(int32_t x, int32_t y)
			: m_X(x), m_Y(y)
		{
		}

		int32_t GetX() const { return m_X; }
		int32_t GetY() const { return m_Y; }

		std::string ToString() const override { return std::format("WindowMovedEvent: {}, {}", m_X, m_Y); }

		OD_EVENT_CLASS_TYPE(WindowMoved)
		OD_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int32_t m_X;
		int32_t m_Y;
	};

	// Files dropped onto the window (eg. assets dragged into the editor).
	class WindowFileDropEvent : public Event
	{
	public:
		explicit WindowFileDropEvent(std::vector<std::filesystem::path> paths)
			: m_Paths(std::move(paths))
		{
		}

		const std::vector<std::filesystem::path>& GetPaths() const { return m_Paths; }

		std::string ToString() const override { return std::format("WindowFileDropEvent: {} file(s)", m_Paths.size()); }

		OD_EVENT_CLASS_TYPE(WindowFileDrop)
		OD_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		std::vector<std::filesystem::path> m_Paths;
	};

}
