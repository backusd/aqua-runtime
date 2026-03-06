# Copilot Instructions for Aqua Runtime

This repository implements the **Aqua Runtime**, a portable C++23 runtime designed
to run deterministic applications across platforms including WebAssembly and
native desktop environments.

Copilot should follow the guidelines below when generating code.

---

# Language Standard

All code must target:

C++23

Use modern language features where appropriate.

---

# Core Principles

Generated code should follow these priorities:

1. Correctness
2. Readability
3. Determinism
4. Minimal dependencies
5. Modern C++ best practices

---

# Required C++ Practices

Prefer:

- `const` correctness
- `constexpr` where possible
- `noexcept` when functions cannot throw
- `[[nodiscard]]` for functions returning important values
- `std::unique_ptr` for ownership
- `std::span` instead of raw pointer + length
- `std::optional` instead of nullable values
- `enum class` instead of plain enums

Avoid:

- macros
- global mutable state
- raw owning pointers
- C-style casts
- unnecessary dynamic allocation

---

# Error Handling

Prefer:

- returning error types
- `std::optional`
- `std::expected` (if available)

Avoid exceptions unless necessary.

---

# Code Organization

Public headers must be placed in:

```
include/aqua/
```

Implementation files must be placed in:

```
src/
```

Header files should contain **only declarations** where possible.

---

# Header Style

Headers must:

- use `#pragma once`
- avoid including unnecessary headers
- prefer forward declarations
- minimize compile-time dependencies

---

# Naming Conventions

Types:

```
PascalCase
```

Functions:

```
snake_case
```

Variables:

```
snake_case
```

Private members:

```
member_name_
```

Namespaces:

```
aqua
aqua::runtime
aqua::events
aqua::platform
```

---

# Determinism Requirements

The runtime is designed to support deterministic execution.

Therefore code must:

- avoid hidden global state
- avoid time queries directly from the OS
- avoid nondeterministic containers where ordering matters

---

# Platform Abstraction

This repository must **not include platform-specific code**.

All platform integrations must occur in external repositories such as:

- aqua-platform-web
- aqua-platform-desktop (future)

The runtime should interact with platforms only through **abstract interfaces**.

---

# Documentation

Public APIs should include brief documentation comments.

Example:

```cpp
/// Processes a runtime event.
void handle_event(const Event& event) noexcept;
```

---

# Testing

Testable components should be designed to allow unit testing.

Avoid tightly coupling code to platform implementations.

---

# Style Goal

The final code should resemble the style of:

- LLVM
- Chromium
- modern C++ systems libraries

Focus on:

- clarity
- small well-defined classes
- minimal coupling