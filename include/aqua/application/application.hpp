
#pragma once

#include "aqua/events/event.hpp"

namespace aqua {

class Application
{
public:
	virtual ~Application() = default;

	virtual void on_event(const events::Event& event) noexcept;
	virtual void update(double delta_time) noexcept;
	virtual void render() noexcept;
};

} // namespace aqua

