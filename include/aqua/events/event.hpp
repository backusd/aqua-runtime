
#pragma once

#include <cstdint>
#include <variant>

namespace aqua::events {

enum class EventType : std::uint8_t
{
	Unknown = 0,
	FrameStart,
	Quit,

	MouseMove,
	MouseButton,
	Key,

	WindowResize,
	Lifecycle,
};

enum class MouseButton : std::uint8_t
{
	Left = 0,
	Middle,
	Right,
	X1,
	X2,
};

enum class ButtonState : std::uint8_t
{
	Pressed = 0,
	Released,
};

enum class KeyState : std::uint8_t
{
	Pressed = 0,
	Released,
};

enum class LifecycleState : std::uint8_t
{
	Started = 0,
	Suspended,
	Resumed,
	Stopped,
};

struct MouseMoveEvent
{
	double x{0.0};
	double y{0.0};
};

struct MouseButtonEvent
{
	MouseButton button{MouseButton::Left};
	ButtonState state{ButtonState::Released};
};

struct KeyEvent
{
	std::uint32_t key_code{0};
	KeyState state{KeyState::Released};
};

struct WindowResizeEvent
{
	std::uint32_t width{0};
	std::uint32_t height{0};
};

struct LifecycleEvent
{
	LifecycleState state{LifecycleState::Started};
};

using EventPayload = std::variant<std::monostate,
	MouseMoveEvent,
	MouseButtonEvent,
	KeyEvent,
	WindowResizeEvent,
	LifecycleEvent>;

struct Event
{
	EventType type{EventType::Unknown};
	EventPayload payload{};

	bool handled{false};

	constexpr void mark_handled() noexcept
	{
		handled = true;
	}

	[[nodiscard]] constexpr bool is_handled() const noexcept
	{
		return handled;
	}

	[[nodiscard]] static constexpr Event quit() noexcept
	{
		return Event{EventType::Quit, std::monostate{}};
	}

	[[nodiscard]] static constexpr Event frame_start() noexcept
	{
		return Event{EventType::FrameStart, std::monostate{}};
	}

	[[nodiscard]] static constexpr Event mouse_move(double x, double y) noexcept
	{
		return Event{EventType::MouseMove, MouseMoveEvent{x, y}};
	}

	[[nodiscard]] static constexpr Event mouse_button(MouseButton button, ButtonState state) noexcept
	{
		return Event{EventType::MouseButton, MouseButtonEvent{button, state}};
	}

	[[nodiscard]] static constexpr Event key(std::uint32_t key_code, KeyState state) noexcept
	{
		return Event{EventType::Key, KeyEvent{key_code, state}};
	}

	[[nodiscard]] static constexpr Event window_resize(std::uint32_t width, std::uint32_t height) noexcept
	{
		return Event{EventType::WindowResize, WindowResizeEvent{width, height}};
	}

	[[nodiscard]] static constexpr Event lifecycle(LifecycleState state) noexcept
	{
		return Event{EventType::Lifecycle, LifecycleEvent{state}};
	}
};

} // namespace aqua::events

