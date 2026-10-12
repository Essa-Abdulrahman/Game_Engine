#include <doctest/doctest.h>

#include "Odyssey/Events/ApplicationEvent.h"
#include "Odyssey/Events/KeyEvent.h"
#include "Odyssey/Events/MouseEvent.h"

#include <string>

using namespace Odyssey;

TEST_CASE("Events: static and dynamic types agree")
{
	KeyPressedEvent keyPressed(KeyCode::A);
	WindowResizeEvent resize(1280, 720);

	CHECK(keyPressed.GetEventType() == KeyPressedEvent::GetStaticType());
	CHECK(resize.GetEventType() == WindowResizeEvent::GetStaticType());
	CHECK(std::string(keyPressed.GetName()) == "KeyPressed");
	CHECK(std::string(resize.GetName()) == "WindowResize");
}

TEST_CASE("Events: categories")
{
	const MouseButtonPressedEvent mouseButton(MouseCode::Left);
	CHECK(mouseButton.IsInCategory(EventCategoryMouse));
	CHECK(mouseButton.IsInCategory(EventCategoryMouseButton));
	CHECK(mouseButton.IsInCategory(EventCategoryInput));
	CHECK_FALSE(mouseButton.IsInCategory(EventCategoryKeyboard));

	const KeyTypedEvent typed(0x41);
	CHECK(typed.IsInCategory(EventCategoryKeyboard));
	CHECK(typed.IsInCategory(EventCategoryInput));

	const WindowCloseEvent close;
	CHECK(close.IsInCategory(EventCategoryApplication));
	CHECK_FALSE(close.IsInCategory(EventCategoryInput));
}

TEST_CASE("Events: dispatcher only invokes handlers for the matching type")
{
	KeyPressedEvent event(KeyCode::Space, true);
	EventDispatcher dispatcher(event);

	bool resizeCalled = false;
	const bool resizeDispatched = dispatcher.Dispatch<WindowResizeEvent>([&](WindowResizeEvent&)
	{
		resizeCalled = true;
		return true;
	});
	CHECK_FALSE(resizeDispatched);
	CHECK_FALSE(resizeCalled);
	CHECK_FALSE(event.Handled);

	KeyCode receivedKey = KeyCode::A;
	bool receivedRepeat = false;
	const bool keyDispatched = dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& keyEvent)
	{
		receivedKey = keyEvent.GetKeyCode();
		receivedRepeat = keyEvent.IsRepeat();
		return false;
	});
	CHECK(keyDispatched);
	CHECK(receivedKey == KeyCode::Space);
	CHECK(receivedRepeat);
	CHECK_FALSE(event.Handled);
}

TEST_CASE("Events: handled state is sticky")
{
	WindowCloseEvent event;
	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<WindowCloseEvent>([](WindowCloseEvent&) { return true; });
	CHECK(event.Handled);

	dispatcher.Dispatch<WindowCloseEvent>([](WindowCloseEvent&) { return false; });
	CHECK(event.Handled);
}

TEST_CASE("Events: ToString describes the payload")
{
	CHECK(WindowResizeEvent(800, 600).ToString() == "WindowResizeEvent: 800, 600");
	CHECK(WindowMovedEvent(-10, 20).ToString() == "WindowMovedEvent: -10, 20");
	CHECK(KeyPressedEvent(KeyCode::Escape).ToString() == "KeyPressedEvent: Escape (repeat = false)");
	CHECK(KeyReleasedEvent(KeyCode::Enter).ToString() == "KeyReleasedEvent: Enter");
	CHECK(KeyTypedEvent(0x263A).ToString() == "KeyTypedEvent: U+263A");
	CHECK(MouseButtonPressedEvent(MouseCode::Right).ToString() == "MouseButtonPressedEvent: Right");
	CHECK(MouseButtonReleasedEvent(MouseCode::Button4).ToString() == "MouseButtonReleasedEvent: Button4");
	CHECK(MouseMovedEvent(1.5f, 2.0f).ToString() == "MouseMovedEvent: 1.5, 2");
	CHECK(MouseScrolledEvent(0.0f, -1.0f).ToString() == "MouseScrolledEvent: 0, -1");
	CHECK(WindowCloseEvent().ToString() == "WindowClose");
}

TEST_CASE("Events: file drop events own their paths")
{
	WindowFileDropEvent event({ "Assets/Helmet.gltf", "Assets/Sky.hdr" });
	REQUIRE(event.GetPaths().size() == 2);
	CHECK(event.GetPaths()[0] == std::filesystem::path("Assets/Helmet.gltf"));
	CHECK(event.ToString() == "WindowFileDropEvent: 2 file(s)");
}
