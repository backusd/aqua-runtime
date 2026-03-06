# Aqua Runtime Architecture

## Overview

**Aqua Runtime** is a portable C++23 runtime designed to host deterministic applications
that can run across multiple platforms including the browser (WebAssembly) and native
desktop environments.

The runtime is responsible for:

- Application lifecycle management
- Event dispatching
- Frame scheduling
- Input abstraction
- Platform-independent interfaces

Platform-specific functionality (browser, desktop, etc.) is implemented in separate
repositories such as:

- aqua-platform-web
- aqua-platform-desktop (future)

This repository intentionally contains **no platform-specific code**.

---

# Design Goals

The runtime is designed around several principles:

### 1. Platform Independence

The runtime must compile for:

- WebAssembly
- Linux
- macOS
- Windows

All platform-specific functionality is implemented behind platform interfaces.

---

### 2. Deterministic Execution

Aqua is designed to support deterministic simulation. This enables:

- Multiplayer lockstep networking
- Replay systems
- Debug determinism

The runtime therefore:

- Avoids hidden global state
- Controls time explicitly
- Ensures predictable event ordering

---

### 3. Clear Subsystem Boundaries

Subsystems communicate through **well-defined interfaces**.

Example subsystems:

```
Application
Event System
Renderer (future subsystem)
Physics (future subsystem)
Networking (future subsystem)
```

Each subsystem should:

- Have a minimal public interface
- Hide implementation details
- Avoid cyclic dependencies

---

### 4. Explicit Ownership

Memory ownership must always be obvious.

Preferred ownership model:

- `std::unique_ptr` for exclusive ownership
- `std::shared_ptr` only when required
- references for non-owning access

Raw pointers should **never imply ownership**.

---

### 5. Minimal Dependencies

The runtime should avoid third-party dependencies whenever possible.

Allowed dependencies:

- C++23 standard library

All other functionality should be implemented internally or placed in
separate optional subsystems.

---

# Core Runtime Components

## Application

The `Application` represents the root of a running Aqua program.

Responsibilities:

- Lifecycle management
- Event processing
- Frame updates
- Coordination of subsystems

Typical flow:

```
initialize()
run loop:
    process events
    update
    render
shutdown()
```

---

## Event System

Events originate from the platform layer (browser, desktop, etc).

Examples:

- Mouse events
- Keyboard events
- Window resize
- Application lifecycle

The runtime defines platform-independent event types.

Platform implementations translate native events into Aqua events.

---

## Platform Interface

Platform implementations provide functionality such as:

- OS-level event polling
- Time
- Window creation
- Input events
- Frame scheduling
- Graphics context

Example platform implementations:

```
aqua-platform-web
aqua-platform-desktop
```

The runtime communicates with platforms only through abstract interfaces.

### Platform vs Window

The runtime splits OS integration into two concepts:

- **Platform**: OS-level services (event pumping, time, window creation)
- **Window**: an individual window instance

This keeps the Platform API small, and prepares the runtime for:

- multiple windows
- headless operation
- renderer attachments per-window (future)

---

## Window Interface

The `Window` interface represents an individual window instance created by a platform implementation.

Responsibilities:

- expose size (width/height)
- provide minimal window controls (e.g. title)

The runtime owns no platform-specific windowing code; window instances are created by platform repositories and returned to the runtime/application via `std::unique_ptr`.

---

## Frame Execution Model

Aqua applications operate using a frame-based execution model.

Each frame executes:

```
1. Platform Poll Events
2. Emit FrameStart Event
3. Process Events
4. Update Application State
5. Render
```

This model works consistently across:

- browsers (requestAnimationFrame)
- desktop event loops

---

# Threading Model

The runtime currently assumes a **single-threaded execution model**.

Reasons:

- Browser environments are largely single-threaded
- Deterministic simulations are easier to reason about
- Simplifies portability

Future subsystems may introduce worker threads where appropriate.

---

# Directory Structure

The repository follows this layout:

```
include/aqua/
    application/
    events/
    platform/

src/
    application/
    events/
    runtime/

docs/
    architecture.md

tests/
```

Guidelines:

- Public headers live in `include/`
- Implementation files live in `src/`
- Platform code must never be introduced here

---

# Example Control Flow

Browser platform example:

```
JavaScript Event
    ↓
aqua-platform-web
    ↓
Aqua Event
    ↓
Runtime Event Queue
    ↓
Application::on_event()
```

---

# Future Subsystems

The runtime is designed to support additional subsystems:

### Renderer

A cross-platform rendering abstraction supporting:

- WebGL
- WebGPU
- Vulkan

### Physics

Deterministic physics simulation.

### Networking

Deterministic lockstep networking.

---

# Non-Goals

The runtime is **not intended to be**:

- A full game engine
- A UI framework
- A web framework

It is a **low-level deterministic runtime** upon which such systems could be built.

---

# Coding Standards

The project follows modern C++23 practices:

- Prefer `const` correctness
- Use `constexpr` when appropriate
- Mark functions `noexcept` where possible
- Use `[[nodiscard]]` for functions returning important results
- Avoid macros
- Avoid global state
- Prefer value semantics

---

# Summary

Aqua Runtime provides a deterministic, portable C++ runtime that separates:

- platform code
- application logic
- future subsystems

The architecture prioritizes:

- portability
- determinism
- simplicity
- modern C++ design
