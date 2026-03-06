
#include "aqua/events/event_queue.hpp"

namespace aqua::events {

void EventQueue::push(Event event)
{
	m_queue.push_back(std::move(event));
}

std::optional<Event> EventQueue::pop() noexcept
{
	if (m_queue.empty())
	{
		return std::nullopt;
	}

	auto event = std::move(m_queue.front());
	m_queue.pop_front();
	return event;
}

bool EventQueue::empty() const noexcept
{
	return m_queue.empty();
}

std::size_t EventQueue::size() const noexcept
{
	return m_queue.size();
}

void EventQueue::clear() noexcept
{
	m_queue.clear();
}

} // namespace aqua::events

