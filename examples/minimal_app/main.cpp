
#include "aqua/runtime/runtime.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>

namespace {

class MinimalApplication final : public aqua::Application
{
public:
	void on_event(const aqua::events::Event& event) noexcept override
	{
		if (event.type == aqua::events::EventType::Quit)
		{
			std::cout << "Received Quit event\n";
		}
	}

	void update(double delta_time) noexcept override
	{
		++frames;
		std::cout << "update(dt=" << delta_time << ") frame=" << frames << "\n";
	}

	void render() noexcept override
	{
		std::cout << "render()\n";
	}

	std::size_t frames{0};
};

class MinimalPlatform final : public aqua::Platform
{
public:
	MinimalPlatform() : m_start(std::chrono::steady_clock::now()) {}

	void set_event_queue(aqua::events::EventQueue* queue) noexcept override { m_queue = queue; }

	void poll_events() noexcept override
	{
		if (!m_queue)
		{
			return;
		}

		// Minimal portable behavior: quit after a couple frames.
		++m_poll_count;
		if (m_poll_count == 3)
		{
			m_queue->push(aqua::events::Event::quit());
		}
	}

	[[nodiscard]] double time() const noexcept override
	{
		using Seconds = std::chrono::duration<double>;
		const auto now = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<Seconds>(now - m_start).count();
	}

private:
	aqua::events::EventQueue* m_queue{nullptr};
	std::chrono::steady_clock::time_point m_start{};
	std::size_t m_poll_count{0};
};

} // namespace

int main()
{
	auto platform = std::make_unique<MinimalPlatform>();
	auto app = std::make_unique<MinimalApplication>();
	aqua::Runtime runtime{std::move(platform), std::move(app)};
	runtime.run();
	return 0;
}

