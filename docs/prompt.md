Modify the Aqua Runtime architecture to split the Platform abstraction
into two separate concepts:

    Platform  -> OS services and event pumping
    Window    -> individual window instance

This pattern is used in professional engines to keep platform APIs small
and to support multiple windows and headless operation in the future.

Before generating code, re-read:

    docs/architecture.md

Follow the architecture described there and update it where necessary.

Changes I made that you need to be aware of:
- In event.hpp, I added a "bool handled" flag to "struct Event" as well as methods "mark_handled" and "is_handled"

==================================================
GOAL
==================================================

Introduce a new Window interface and update Platform so that it creates
Window instances rather than representing the window itself.

This change prepares the runtime for:

- multiple windows
- headless mode
- renderer attachments to windows
- better separation between OS services and window management


==================================================
NEW FILES TO CREATE
==================================================

Create a new public header:

include/aqua/platform/window.hpp

The interface should look roughly like:

namespace aqua::platform {

class Window
{
public:
    virtual ~Window() = default;

    [[nodiscard]] virtual std::uint32_t width() const noexcept = 0;
    [[nodiscard]] virtual std::uint32_t height() const noexcept = 0;

    virtual void set_title(const char* title) noexcept = 0;
};

}

This interface must be completely platform-independent.


==================================================
MODIFY PLATFORM INTERFACE
==================================================

Update:

include/aqua/platform/platform.hpp


Platform should represent OS-level services:

- event polling
- time
- window creation


Updated interface should include:

class Platform
{
public:
    virtual ~Platform() = default;

    virtual void set_event_queue(events::EventQueue*) noexcept = 0;

    virtual void poll_events() noexcept = 0;

    [[nodiscard]] virtual double time() const noexcept = 0;

    [[nodiscard]] virtual std::unique_ptr<platform::Window>
    create_window(std::uint32_t width,
                  std::uint32_t height,
                  const char* title) = 0;
};


Important:

- Platform owns no windows
- Windows are returned as std::unique_ptr
- The runtime or application may store the window


==================================================
ADD FRAMESTART EVENT
==================================================

Add a new event type:

EventType::FrameStart

This should be emitted once at the beginning of each frame.

Why this is important:

Some platforms (especially browsers) do not allow the runtime to drive
the frame loop directly. Instead, the platform triggers frames through
a callback such as requestAnimationFrame.

By introducing a FrameStart event now, the runtime can later support
both models:

Runtime-driven loop:

    poll_events
    process_events
    update
    render

Platform-driven loop:

    platform signals FrameStart
    runtime performs update/render

For now:

Runtime should enqueue FrameStart at the beginning of each frame before
update() is called.

Applications may ignore it for now.


==================================================
EVENT SYSTEM CHANGES
==================================================

Modify:

include/aqua/events/event.hpp

Add:

    EventType::FrameStart

FrameStart should not require a payload.


==================================================
RUNTIME LOOP CHANGE
==================================================

Update Runtime::run() so that each frame does:

    platform->poll_events()

    enqueue FrameStart event

    process_events()

    application->update(delta_time)

    application->render()


FrameStart should be delivered through the existing event queue.


==================================================
README UPDATES
==================================================

Update README.md in the following ways:

1. Architecture section

Add description of the new Window interface and its relationship to
Platform.

Explain that:

    Platform = OS integration
    Window   = window instance

2. Frame model section

Update the frame sequence to:

    Platform::poll_events()
    FrameStart event emitted
    Runtime::process_events()
    Application::update()
    Application::render()

3. Platform integration section

Document that platform implementations must implement BOTH:

    Platform
    Window

and that Platform::create_window() returns a platform-specific Window.

Example explanation:

Win32Platform -> creates Win32Window
CocoaPlatform -> creates CocoaWindow
WebPlatform   -> creates BrowserWindow


==================================================
CODING REQUIREMENTS
==================================================

Continue following the existing code style and design:

- C++23
- const correctness
- constexpr where appropriate
- noexcept where appropriate
- [[nodiscard]] where appropriate
- avoid macros
- avoid global state
- use std::unique_ptr for ownership
- maintain clear header/source separation


==================================================
DO NOT CHANGE
==================================================

Do NOT modify the following concepts:

- Application interface
- EventQueue API
- the basic Event struct design
- repository directory structure
- namespace layout
- build system (CMake)
- deterministic single-threaded model

Do NOT introduce:

- platform-specific headers
- OS APIs
- windowing libraries
- threading primitives


==================================================
EXPECTED RESULT
==================================================

After this change:

- Platform provides OS services
- Window represents a window instance
- Runtime emits FrameStart each frame
- Platform repos will implement both Platform and Window
- Aqua Runtime remains fully platform-independent