Generate the initial runtime skeleton for the Aqua Runtime.

The Aqua Runtime is a C++23 portable runtime that runs deterministic
applications across multiple platforms including WebAssembly and native
desktop environments.

Important design requirements:

- This repository contains NO platform specific code.
- Platforms are implemented in separate repositories such as aqua-platform-web.
- The runtime must remain fully platform independent.

Documentation:

- The architecture for this project is documented in docs/architecture.md
- Re-read docs/architecture.md before generating any code

Changes I made that you need to be aware of:

- In the root CMakeLists.txt, I set the default AQUA_RUNTIME_BUILD_TESTS and AQUA_RUNTIME_BUILD_EXAMPLES values to OFF


Architecture overview:

Core components:

1. Application
   Represents the root application instance.
   Responsible for lifecycle management and frame updates.

2. Event System
   Defines platform-independent events such as:
   - mouse events
   - keyboard events
   - window resize events
   - lifecycle events

3. Platform Interface
   The runtime interacts with platforms through an abstract interface.
   Platform implementations will translate native events into Aqua events.

4. Runtime
   Coordinates the application, event queue, and platform.

Frame model:

Each frame executes in this order:

    process events
    update application
    render

Coding requirements:

- Use C++23
- Use modern C++ best practices
- Use const correctness
- Use constexpr where appropriate
- Use noexcept where possible
- Use [[nodiscard]] where appropriate
- Avoid macros
- Avoid global state
- Prefer std::unique_ptr for ownership
- Use enum class for events
- Use std::variant or a clean type system for event payloads

File organization:

Public headers go in:

    include/aqua/

Implementation files go in:

    src/

Create the following components:

include/aqua/application/application.hpp
include/aqua/events/event.hpp
include/aqua/events/event_queue.hpp
include/aqua/platform/platform.hpp
include/aqua/runtime/runtime.hpp

src/application.cpp
src/event_queue.cpp
src/runtime.cpp

Class design:

Application

class Application
{
public:
    virtual ~Application() = default;

    virtual void on_event(const Event& event) noexcept;
    virtual void update(double delta_time) noexcept;
    virtual void render() noexcept;
};

Event

Define a type-safe event system with:

enum class EventType

Event struct containing event type and payload.

EventQueue

Thread-safe not required yet.

Simple FIFO queue.

Platform

Abstract interface implemented by platform layers.

class Platform
{
public:
    virtual ~Platform() = default;

    virtual void poll_events() noexcept = 0;
    virtual double time() const noexcept = 0;
};

Runtime

Coordinates the system.

class Runtime
{
public:
    Runtime(std::unique_ptr<Platform> platform,
            std::unique_ptr<Application> application);

    void run() noexcept;

private:
    void process_events() noexcept;

    std::unique_ptr<Platform> m_platform;
    std::unique_ptr<Application> m_application;

    EventQueue m_event_queue;
};

Runtime::run() should:

while running:

    platform->poll_events()
    process_events()
    application->update(delta_time)
    application->render()

Design the code to be minimal but clean and extensible.

All code should compile and follow modern C++ design practices.