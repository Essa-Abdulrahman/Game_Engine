#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

// Note: the list parameter is called ENTRY (not X) because X is itself a key name.
// Single source of truth for key names and values. Values match GLFW 3.4 (GLFW_KEY_*) so the platform layer
// can translate with a cast; the GLFW backend static_asserts this. The list also generates the string tables
// used by scripting, serialization and the agent protocol, which keeps names and values from drifting apart.
#define OD_KEY_CODE_LIST(ENTRY) \
	ENTRY(Space, 32) ENTRY(Apostrophe, 39) ENTRY(Comma, 44) ENTRY(Minus, 45) ENTRY(Period, 46) ENTRY(Slash, 47) \
	ENTRY(D0, 48) ENTRY(D1, 49) ENTRY(D2, 50) ENTRY(D3, 51) ENTRY(D4, 52) ENTRY(D5, 53) ENTRY(D6, 54) ENTRY(D7, 55) ENTRY(D8, 56) ENTRY(D9, 57) \
	ENTRY(Semicolon, 59) ENTRY(Equal, 61) \
	ENTRY(A, 65) ENTRY(B, 66) ENTRY(C, 67) ENTRY(D, 68) ENTRY(E, 69) ENTRY(F, 70) ENTRY(G, 71) ENTRY(H, 72) ENTRY(I, 73) ENTRY(J, 74) ENTRY(K, 75) ENTRY(L, 76) ENTRY(M, 77) \
	ENTRY(N, 78) ENTRY(O, 79) ENTRY(P, 80) ENTRY(Q, 81) ENTRY(R, 82) ENTRY(S, 83) ENTRY(T, 84) ENTRY(U, 85) ENTRY(V, 86) ENTRY(W, 87) ENTRY(X, 88) ENTRY(Y, 89) ENTRY(Z, 90) \
	ENTRY(LeftBracket, 91) ENTRY(Backslash, 92) ENTRY(RightBracket, 93) ENTRY(GraveAccent, 96) ENTRY(World1, 161) ENTRY(World2, 162) \
	ENTRY(Escape, 256) ENTRY(Enter, 257) ENTRY(Tab, 258) ENTRY(Backspace, 259) ENTRY(Insert, 260) ENTRY(Delete, 261) \
	ENTRY(Right, 262) ENTRY(Left, 263) ENTRY(Down, 264) ENTRY(Up, 265) ENTRY(PageUp, 266) ENTRY(PageDown, 267) ENTRY(Home, 268) ENTRY(End, 269) \
	ENTRY(CapsLock, 280) ENTRY(ScrollLock, 281) ENTRY(NumLock, 282) ENTRY(PrintScreen, 283) ENTRY(Pause, 284) \
	ENTRY(F1, 290) ENTRY(F2, 291) ENTRY(F3, 292) ENTRY(F4, 293) ENTRY(F5, 294) ENTRY(F6, 295) ENTRY(F7, 296) ENTRY(F8, 297) ENTRY(F9, 298) ENTRY(F10, 299) \
	ENTRY(F11, 300) ENTRY(F12, 301) ENTRY(F13, 302) ENTRY(F14, 303) ENTRY(F15, 304) ENTRY(F16, 305) ENTRY(F17, 306) ENTRY(F18, 307) ENTRY(F19, 308) \
	ENTRY(F20, 309) ENTRY(F21, 310) ENTRY(F22, 311) ENTRY(F23, 312) ENTRY(F24, 313) ENTRY(F25, 314) \
	ENTRY(KP0, 320) ENTRY(KP1, 321) ENTRY(KP2, 322) ENTRY(KP3, 323) ENTRY(KP4, 324) ENTRY(KP5, 325) ENTRY(KP6, 326) ENTRY(KP7, 327) ENTRY(KP8, 328) ENTRY(KP9, 329) \
	ENTRY(KPDecimal, 330) ENTRY(KPDivide, 331) ENTRY(KPMultiply, 332) ENTRY(KPSubtract, 333) ENTRY(KPAdd, 334) ENTRY(KPEnter, 335) ENTRY(KPEqual, 336) \
	ENTRY(LeftShift, 340) ENTRY(LeftControl, 341) ENTRY(LeftAlt, 342) ENTRY(LeftSuper, 343) \
	ENTRY(RightShift, 344) ENTRY(RightControl, 345) ENTRY(RightAlt, 346) ENTRY(RightSuper, 347) ENTRY(Menu, 348)

// Values match GLFW 3.4 (GLFW_MOUSE_BUTTON_*).
#define OD_MOUSE_CODE_LIST(ENTRY) \
	ENTRY(Left, 0) ENTRY(Right, 1) ENTRY(Middle, 2) ENTRY(Button3, 3) ENTRY(Button4, 4) ENTRY(Button5, 5) ENTRY(Button6, 6) ENTRY(Button7, 7)

namespace Odyssey {

#define OD_INTERNAL_ENUM_ENTRY(name, value) name = value,

	enum class KeyCode : uint16_t
	{
		OD_KEY_CODE_LIST(OD_INTERNAL_ENUM_ENTRY)
	};

	enum class MouseCode : uint16_t
	{
		OD_MOUSE_CODE_LIST(OD_INTERNAL_ENUM_ENTRY)
	};

#undef OD_INTERNAL_ENUM_ENTRY

	// Returns the enumerator name (eg. "Space", "D1", "KPEnter"), or "Unknown" for values outside the enumeration.
	const char* KeyCodeToString(KeyCode keyCode);
	// Case-insensitive lookup by enumerator name.
	std::optional<KeyCode> KeyCodeFromString(std::string_view name);
	// Every key code, in ascending value order.
	std::span<const KeyCode> GetAllKeyCodes();

	const char* MouseCodeToString(MouseCode mouseCode);
	std::optional<MouseCode> MouseCodeFromString(std::string_view name);
	std::span<const MouseCode> GetAllMouseCodes();

}
