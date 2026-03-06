
#include "aqua/runtime/runtime.hpp"

#include <utility>

namespace aqua {

Runtime::Runtime(std::unique_ptr<Platform> platform, std::unique_ptr<Application> application)
	: m_platform(std::move(platform))
	, m_application(std::move(application))
{
	if (m_platform)
	{
		m_platform->set_event_queue(&m_event_queue);
	}
}

void Runtime::run() noexcept
{
	if (!m_platform || !m_application)
	{
		return;
	}

	m_running = true;
	double last_time = m_platform->time();

	while (m_running)
	{
		m_platform->poll_events();
		m_event_queue.push(events::Event::frame_start());
		process_events();

		if (!m_running)
		{
			break;
		}

		const double now = m_platform->time();
		const double delta_time = (now >= last_time) ? (now - last_time) : 0.0;
		last_time = now;

		m_application->update(delta_time);
		m_application->render();
	}
}

void Runtime::process_events() noexcept
{
	while (true)
	{
		auto event_opt = m_event_queue.pop();
		if (!event_opt.has_value())
		{
			break;
		}

		const auto& event = *event_opt;
		if (event.type == events::EventType::Quit)
		{
			m_running = false;
		}

		m_application->on_event(event);
	}
}

} // namespace aqua

