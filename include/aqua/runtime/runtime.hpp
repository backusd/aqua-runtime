
#pragma once

#include <memory>

#include "aqua/application/application.hpp"
#include "aqua/events/event_queue.hpp"
#include "aqua/platform/platform.hpp"

namespace aqua {

class Runtime
{
public:
	Runtime(std::unique_ptr<Platform> platform, std::unique_ptr<Application> application);

	void run() noexcept;

private:
	void process_events() noexcept;

	bool m_running{true};

	std::unique_ptr<Platform> m_platform;
	std::unique_ptr<Application> m_application;

	events::EventQueue m_event_queue;
};

} // namespace aqua

