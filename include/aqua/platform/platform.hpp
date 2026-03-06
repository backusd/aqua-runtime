
#pragma once

#include <cstdint>
#include <memory>

#include "aqua/events/event_queue.hpp"
#include "aqua/platform/window.hpp"

namespace aqua {

class Platform
{
public:
	virtual ~Platform() = default;

	// Called by the runtime once to provide the platform an event sink.
	// Platform implementations should enqueue translated Aqua events here.
	virtual void set_event_queue(events::EventQueue* queue) noexcept = 0;

	virtual void poll_events() noexcept = 0;
	[[nodiscard]] virtual double time() const noexcept = 0;

	[[nodiscard]] virtual std::unique_ptr<platform::Window> create_window(
		std::uint32_t width,
		std::uint32_t height,
		const char* title) = 0;
};

} // namespace aqua

