
#include "aqua/runtime/runtime.hpp"

#include <cassert>
#include <cstddef>
#include <memory>

namespace {

class TestApplication final : public aqua::Application
{
public:
	void on_event(const aqua::events::Event& event) noexcept override
	{
		if (event.type == aqua::events::EventType::Quit)
		{
			++quit_events;
		}
	}

	void update(double) noexcept override { ++updates; }
	void render() noexcept override { ++renders; }

	std::size_t quit_events{0};
	std::size_t updates{0};
	std::size_t renders{0};
};

class TestPlatform final : public aqua::Platform
{
public:
	void set_event_queue(aqua::events::EventQueue* queue) noexcept override { m_queue = queue; }

	void poll_events() noexcept override
	{
		if (!m_queue)
		{
			return;
		}

		if (!m_sent_quit)
		{
			m_queue->push(aqua::events::Event::quit());
			m_sent_quit = true;
		}
	}

	[[nodiscard]] double time() const noexcept override { return 0.0; }

private:
	aqua::events::EventQueue* m_queue{nullptr};
	bool m_sent_quit{false};
};

} // namespace

int main()
{
	auto platform = std::make_unique<TestPlatform>();
	auto app = std::make_unique<TestApplication>();
	auto* app_raw = app.get();

	aqua::Runtime runtime{std::move(platform), std::move(app)};
	runtime.run();

	assert(app_raw->quit_events == 1);
	assert(app_raw->updates == 0);
	assert(app_raw->renders == 0);
	return 0;
}

