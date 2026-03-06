
#pragma once

#include <cstddef>
#include <deque>
#include <optional>

#include "aqua/events/event.hpp"

namespace aqua::events {

class EventQueue
{
public:
	void push(Event event);

	[[nodiscard]] std::optional<Event> pop() noexcept;

	[[nodiscard]] bool empty() const noexcept;
	[[nodiscard]] std::size_t size() const noexcept;

	void clear() noexcept;

private:
	std::deque<Event> m_queue;
};

} // namespace aqua::events

