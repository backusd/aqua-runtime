# Aqua Runtime

Aqua Runtime is a portable **C++23** runtime for hosting **deterministic** applications across multiple platforms (native and WebAssembly).

This repository intentionally contains **no platform-specific code**. Platform implementations live in separate repositories (for example: `aqua-platform-web`).

## Table of contents

- [Goals](#goals)
- [Non-goals](#non-goals)
- [High-level architecture](#high-level-architecture)
- [Frame model](#frame-model)
- [Integration pattern (platform repos)](#integration-pattern-platform-repos)
- [Public API overview](#public-api-overview)
- [Using as a dependency](#using-as-a-dependency)
- [Build](#build)
- [Tests](#tests)
- [Examples](#examples)
- [Repository layout](#repository-layout)
- [Determinism notes](#determinism-notes)
- [License](#license)

## Goals

- **Platform independence**: the runtime compiles for WebAssembly and desktop targets without embedding any OS/browser APIs.
- **Deterministic execution**: predictable ordering and explicit time.
- **Clear subsystem boundaries**: platforms translate native events into Aqua events; apps consume Aqua events.
- **Explicit ownership**: ownership is expressed with `std::unique_ptr` and references.
- **Minimal dependencies**: standard library only.

For the design intent, see [docs/architecture.md](docs/architecture.md).

## Non-goals

- Implementing windows, graphics contexts, audio devices, etc. (that belongs in platform repos).
- A full engine stack (renderer/physics/networking are future subsystems).
- Multi-threading (current model is single-threaded by design).

## High-level architecture

The runtime is built from four core concepts:

- **`Application`**: user code; receives events and runs update/render.
- **Event system**: platform-independent event types and payloads.
- **`Platform`**: OS services and event pumping.
- **`Window`**: an individual window instance created by the platform.
- **`Runtime`**: coordinates polling, event processing, and frame steps.

The platform layer is responsible for translating native events (DOM, Win32, Cocoa, X11/Wayland, etc.) into Aqua events and enqueueing them.

## Frame model

Each frame executes in this order:

1. `Platform::poll_events()`
2. `FrameStart` event is emitted (enqueued by the runtime)
3. `Runtime::process_events()` (drains the event queue FIFO)
4. `Application::update(delta_time)`
5. `Application::render()`

The runtime exits its loop when it receives an `EventType::Quit`.

## Integration pattern (platform repos)

Platform integrations are expected to live in their own repositories and link against this runtime library.

### The contract

In your platform repo, implement both:

- [`aqua::Platform`](include/aqua/platform/platform.hpp) (OS services + window creation)
- [`aqua::platform::Window`](include/aqua/platform/window.hpp) (window instance)

- `set_event_queue(events::EventQueue* queue) noexcept`
  - The runtime calls this once during `Runtime` construction.
  - Store the pointer (non-owning) and use it as the **only** way to deliver events to the runtime.
  - Do not assume thread-safety; current model is single-threaded.

- `poll_events() noexcept`
  - Poll the underlying platform for pending native events.
  - Translate each native event into an Aqua [`events::Event`](include/aqua/events/event.hpp).
  - Enqueue translated events using `queue->push(...)`.
  - If your platform has a natural “quit” signal, enqueue `events::Event::quit()`.

- `time() const noexcept -> double`
  - Return a monotonic time value in **seconds**.
  - The runtime computes `delta_time` as `max(0, time() - last_time)`.

- `create_window(width, height, title) -> std::unique_ptr<platform::Window>`
  - Create a new window instance.
  - Platform owns no windows; ownership transfers to the runtime/application.
  - The returned object is platform-specific (e.g. `Win32Window`, `CocoaWindow`, `BrowserWindow`).

### Minimal pseudo-code

```cpp
class MyPlatform final : public aqua::Platform {
public:
	void set_event_queue(aqua::events::EventQueue* q) noexcept override { queue = q; }

	void poll_events() noexcept override {
		// 1) Poll native events
		// 2) Translate -> Aqua
		// 3) queue->push(aqua::events::Event::...)
	}

	double time() const noexcept override {
		// monotonic seconds
		return ...;
	}

	std::unique_ptr<aqua::platform::Window> create_window(
		std::uint32_t width,
		std::uint32_t height,
		const char* title) override {
		return std::make_unique<MyWindow>(width, height, title);
	}

private:
	aqua::events::EventQueue* queue = nullptr; // non-owning
};

class MyWindow final : public aqua::platform::Window {
public:
	std::uint32_t width() const noexcept override { return w; }
	std::uint32_t height() const noexcept override { return h; }
	void set_title(const char* title) noexcept override { /* platform-specific */ }
private:
	std::uint32_t w{};
	std::uint32_t h{};
};
```

### Why the event queue is injected

Passing an `EventQueue*` into the platform keeps the runtime:

- free of platform headers/APIs
- free of global state
- deterministic in event ordering (FIFO)
- easy to embed into different host loops

If you need tighter integration later (frame pacing, blocking waits, vsync, browser RAF), extend the `Platform` interface in a platform-agnostic way.

## Public API overview

### Application

[`aqua::Application`](include/aqua/application/application.hpp) is the user-defined root object.

Override any of the following (all are `noexcept`):

- `on_event(const events::Event&)`
- `update(double delta_time)`
- `render()`

### Events

Events are defined in [`aqua::events`](include/aqua/events/event.hpp) as:

- `enum class EventType`
- `struct Event { EventType type; EventPayload payload; }`
- `EventPayload` is a `std::variant` holding the current set of event payload structs.

Event creation helpers exist on `Event`:

- `Event::frame_start()`
- `Event::quit()`
- `Event::mouse_move(x, y)`
- `Event::mouse_button(button, state)`
- `Event::key(key_code, state)`
- `Event::window_resize(width, height)`
- `Event::lifecycle(state)`

#### Reading payloads

Event payloads are carried in `Event::payload` as a `std::variant`. Application code can read payloads with `std::get_if`:

```cpp
void MyApp::on_event(const aqua::events::Event& e) noexcept {
	if (e.type == aqua::events::EventType::WindowResize) {
		if (const auto* resize = std::get_if<aqua::events::WindowResizeEvent>(&e.payload)) {
			// resize->width, resize->height
		}
	}
}
```

#### Extending the event set

The current event set is intentionally small and platform-agnostic. If you need new event types:

1. Add a new payload struct to [include/aqua/events/event.hpp](include/aqua/events/event.hpp)
2. Add it to `EventPayload` (`std::variant`)
3. Add a new `EventType` enumerator
4. Update platform repos to translate native events into the new Aqua event

Because this changes the public API surface (and potentially ABI), treat event additions as a versioned API change.

### EventQueue

[`aqua::events::EventQueue`](include/aqua/events/event_queue.hpp) is a simple FIFO queue:

- `push(Event)`
- `pop() -> std::optional<Event>`
- `empty()`, `size()`, `clear()`

Thread-safety is intentionally out of scope for now.

### Runtime

[`aqua::Runtime`](include/aqua/runtime/runtime.hpp) wires everything together:

```cpp
auto platform = std::make_unique<MyPlatform>();
auto app      = std::make_unique<MyApp>();

aqua::Runtime runtime{std::move(platform), std::move(app)};
runtime.run();
```

The runtime:

- injects its event queue into the platform
- drains events FIFO and forwards them to `Application::on_event`
- stops when it sees `EventType::Quit`

## Using as a dependency

`aqua-runtime` is a CMake library target. Typical consumption patterns:

### `add_subdirectory`

If you vendor this repo alongside your platform/app:

```cmake
add_subdirectory(external/aqua-runtime)

add_library(my-platform ...)
target_link_libraries(my-platform PRIVATE aqua-runtime)
```

### `FetchContent`

If you want CMake to fetch the runtime:

```cmake
include(FetchContent)

FetchContent_Declare(
	aqua_runtime
	# Replace with the actual repo URL/tag.
	GIT_REPOSITORY https://example.com/your/aqua-runtime.git
	GIT_TAG        main
)

FetchContent_MakeAvailable(aqua_runtime)

target_link_libraries(my-platform PRIVATE aqua-runtime)
```

Note: this repository currently focuses on being a buildable source dependency; it does not yet provide an installable package config.

## Build

### CMake options

This project is built with CMake and a single library target: `aqua-runtime`.

Options (defaults are **OFF**):

- `AQUA_RUNTIME_BUILD_TESTS`
- `AQUA_RUNTIME_BUILD_EXAMPLES`

### Configure + build

```sh
cmake -S . -B build -DAQUA_RUNTIME_BUILD_TESTS=ON -DAQUA_RUNTIME_BUILD_EXAMPLES=ON
cmake --build build --config Debug
```

### CMake presets

If you use [CMakePresets.json](CMakePresets.json), the `default` preset enables tests/examples:

```sh
cmake --preset default
cmake --build --preset default
ctest --preset default
```

## Tests

When enabled (`AQUA_RUNTIME_BUILD_TESTS=ON`), tests build as `aqua-runtime-tests`.

```sh
cd build
ctest -C Debug --output-on-failure
```

## Examples

When enabled (`AQUA_RUNTIME_BUILD_EXAMPLES=ON`), build the minimal example:

- Source: [examples/minimal_app/main.cpp](examples/minimal_app/main.cpp)

This example includes a tiny stub `Platform` that quits after a couple polls.

## Repository layout

```text
include/aqua/        Public headers (platform-independent)
src/                Implementations
docs/               Architecture and design notes
examples/            Small usage examples
tests/               Runtime tests
```

## Determinism notes

Determinism is a cross-cutting goal. The current skeleton supports it by construction:

- **Explicit time**: `delta_time` comes only from `Platform::time()`.
- **Predictable event ordering**: events are processed FIFO per frame.
- **No global runtime state**: state lives in `Runtime`, `Application`, and `Platform` instances.
- **Single-threaded model**: avoids nondeterministic scheduling.

Determinism of your simulation also depends on your application logic. Common pitfalls include:

- using non-deterministic floating point settings across platforms
- relying on OS timers or random sources without a controlled seed
- processing events in an order that depends on container iteration

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE).
