
#pragma once

#include <cstdint>

namespace aqua::platform {

class Window
{
public:
	virtual ~Window() = default;

	[[nodiscard]] virtual std::uint32_t width() const noexcept = 0;
	[[nodiscard]] virtual std::uint32_t height() const noexcept = 0;

	virtual void set_title(const char* title) noexcept = 0;
};

} // namespace aqua::platform
