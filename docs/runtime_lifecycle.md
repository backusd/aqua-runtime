# Aqua Runtime Lifecycle

This document describes the lifecycle of an Aqua application and how the
runtime coordinates platform integration, event processing, and frame
execution.

Understanding the lifecycle is important for platform implementers and
application developers who want to integrate deeply with the runtime.

---

## Overview

The Aqua runtime coordinates four major components:

- **Application** – user-defined application logic
- **Runtime** – execution controller
- **Platform** – operating system integration layer
- **Window** – platform-specific window instance(s)

High-level relationships:

Application
└── Runtime
└── Platform (event pumping, time, window creation)

Window(s)
└── owned by host or application (created by Platform)

Platform implementations translate native OS or browser events into
platform-independent Aqua events.

These events are delivered to the application through the runtime.

---

## Lifecycle phases

The lifecycle of an Aqua application is divided into the following phases:

1. Construction
2. Initialization
3. Frame Loop
4. Shutdown

---

## 1. Construction

The host program constructs the platform and application objects and
passes them to the runtime.

Example:

```cpp
auto platform = std::make_unique<MyPlatform>();
auto app      = std::make_unique<MyApplication>();

aqua::Runtime runtime{
    std::move(platform),
    std::move(app)
};
```

During runtime construction:

1. The runtime creates its internal EventQueue
2. The runtime injects the event queue into the platform using:

```cpp
Platform::set_event_queue(events::EventQueue*)
```

This allows the platform to deliver translated events into the runtime.

Ownership model:

Runtime
├── Platform (unique_ptr)
├── Application (unique_ptr)
└── EventQueue

The platform does not own the event queue.

Note: the runtime does not currently expose accessors for the platform or event queue; after ownership is transferred into `aqua::Runtime`, the host cannot call platform methods through that moved-from pointer.

## 2. Initialization

The current skeleton has no explicit initialization callbacks. Any setup work is performed by the host program (and/or within constructors) before calling `Runtime::run()`.

Windows are optional. If you need one (or multiple), create them via `Platform::create_window()` **before** transferring the platform into the runtime.

Example:

```cpp
auto platform = std::make_unique<MyPlatform>();
auto window   = platform->create_window(1280, 720, "Aqua Application");

auto app = std::make_unique<MyApplication>();

aqua::Runtime runtime{std::move(platform), std::move(app)};
runtime.run();
```

The runtime itself does not require a window, which allows headless execution (e.g. server environments or simulation testing).

## 3. Frame loop

The runtime executes a deterministic frame loop.

Each frame performs the following steps:

```cpp
Platform::poll_events()
// Runtime enqueues FrameStart event
Runtime::process_events()
Application::update(delta_time)
Application::render()
```

Detailed explanation:

### 3.1 Poll platform events

The runtime calls:

```cpp
Platform::poll_events()
```

The platform performs the following:

1. Poll native OS or browser events
2. Translate them into Aqua events
3. Push them into the runtime's EventQueue

Example translations:

| Native event   | Aqua event     |
| -------------- | -------------- |
| `WM_MOUSEMOVE` | `MouseMove`    |
| `WM_KEYDOWN`   | `Key`          |
| window resize  | `WindowResize` |
| window close   | `Quit`         |

The platform must not process application logic.
Its only responsibility is event translation.

### 3.2 FrameStart event

Once per frame, the runtime enqueues a:

```cpp
EventType::FrameStart
```

This event represents the beginning of a new simulation frame.

It exists for future compatibility with platforms that drive frame execution themselves (for example browsers using requestAnimationFrame).

Applications may ignore this event.

Current ordering in the skeleton:

1. `Platform::poll_events()`
2. runtime enqueues `FrameStart`
3. `Runtime::process_events()`

### 3.3 Process events

The runtime drains the event queue in FIFO order.

For each event:

```cpp
Application::on_event(const Event&)
```

is invoked.

This ensures deterministic ordering of event delivery.

Quit behavior: when a `Quit` event is encountered, the runtime marks itself as no longer running, but still forwards the `Quit` event to `Application::on_event`.

Handled flag: `events::Event` contains a `handled` flag (`mark_handled()` / `is_handled()`), but the runtime currently does not branch on it.

### 3.4 Application update

The runtime calls:

```cpp
Application::update(delta_time)
```

delta_time is computed using:

```cpp
delta_time = clamp_non_negative(platform->time() - previous_time)
```

Time is always measured in seconds and must come from a
monotonic clock provided by the platform implementation.

If the platform time goes backwards, the runtime clamps `delta_time` to `0.0`.

### 3.5 Application render

After update:

```cpp
Application::render()
```

is called.

The runtime itself does not perform rendering. Rendering will later
be implemented by a renderer subsystem that integrates with platform
windows.

## 4. Shutdown

Shutdown occurs when a Quit event is received.

Typical sources:

- window close
- OS shutdown
- application request

When a Quit event is processed:

1. The runtime exits the frame loop
2. `Runtime::run()` returns to the host

Runtime object destruction happens later, when the host destroys the `aqua::Runtime` instance.

Current member destruction order (reverse of declaration in `Runtime`):

1. EventQueue destroyed
2. Application destroyed
3. Platform destroyed
